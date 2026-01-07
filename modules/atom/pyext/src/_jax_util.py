import jax
import jax.numpy as jnp
import math
import functools
import IMP.atom
from typing import NamedTuple
from IMP._jax_util import JaxOptimizerInfo


# Conversion from derivatives (in kcal/mol/A) to acceleration (A/fs/fs)
_deriv_to_acceleration = -4.1868e-4


def _propagate_coordinates(X, indexes, mass, time_step, velocity_cap=None):
    linvel = X['linvel'].at[indexes]
    dcoord = X["xyz'"][indexes]
    v = linvel.get() + time_step * 0.5 * dcoord * _deriv_to_acceleration / mass
    if velocity_cap is not None:
        v = jnp.clip(v, -velocity_cap, velocity_cap)
    X['linvel'] = linvel.set(v)
    X['xyz'] = X['xyz'].at[indexes].add(v * time_step)


def _propagate_velocities(X, indexes, mass, time_step):
    linvel = X['linvel'].at[indexes]
    dcoord = X["xyz'"][indexes]
    X['linvel'] = linvel.add(
        time_step * 0.5 * dcoord * _deriv_to_acceleration / mass)


class _MDState(NamedTuple):
    """Track the state of a MolecularDynamics optimization using JAX"""

    # Current model state
    X: dict
    # Number of steps taken
    steps: int


class _MDJaxInfo(JaxOptimizerInfo):
    def __init__(self, md):
        super().__init__(md)
        indexes = md.get_simulation_particle_indexes()
        deriv_func = jax.grad(self.score_func)
        time_step = md.get_maximum_time_step()
        velocity_cap = md.get_velocity_cap()
        # Would like to use math.isfinite here but it is not guaranteed
        # that a C++ "infinite" value is also considered to be math.inf
        if velocity_cap < 1e20:
            velocity_cap = jnp.array([velocity_cap] * 3)
        else:
            velocity_cap = None

        def init_func(X):
            X["xyz'"] = deriv_func(X)["xyz"]
            return _MDState(X=X, steps=0)

        def apply_func(ms):
            X = ms.X
            mass = X['mass'][indexes]
            # Get coordinates at t+(delta t) and velocities at t+(delta t/2)
            _propagate_coordinates(X, indexes, mass, time_step,
                                   velocity_cap)
            # Get new derivatives at t+(delta t)
            X["xyz'"] = deriv_func(X)["xyz"]
            # Get velocities at t+(delta t)
            _propagate_velocities(X, indexes, mass, time_step)
            return ms._replace(steps=ms.steps + 1)

        self.init_func = init_func
        self.apply_func = apply_func

    def get_model_state(self):
        X = super().get_model_state()
        m = self._opt.get_model()
        X['mass'] = m.get_floats_numpy(IMP.atom.Mass.get_mass_key())
        X['linvel'] = jax.numpy.array(
            m.get_vector3ds_numpy(IMP.atom.LinearVelocity.get_velocity_key()))
        X['xyz'] = jax.numpy.array(X['xyz'])
        return X


def _md_optimize(md, max_steps):
    from IMP.core._jax_util import _JAXOptimizer

    jopt = _JAXOptimizer(md, max_steps)
    inner_steps = jopt.inner_steps
    ji = md._get_jax()
    init_func = jax.jit(ji.init_func)
    score_func = jax.jit(ji.score_func)
    apply_func = jax.jit(
        lambda X: jax.lax.fori_loop(0, inner_steps,
                                    lambda i, X: ji.apply_func(X), X))

    md_state = init_func(ji.get_model_state())
    m = md.get_model()
    linvel = m.get_vector3ds_numpy(IMP.atom.LinearVelocity.get_velocity_key())
    xyz = m.get_spheres_numpy()[0]
    dxyz = m.get_sphere_derivatives_numpy()[0]

    for _ in jopt.loop():
        md_state = apply_func(md_state)
        # Resync IMP Model arrays with JAX
        X = md_state.X
        linvel[:] = X['linvel']
        xyz[:] = X['xyz']
        dxyz[:] = X["xyz'"]
    return score_func(md_state.X)

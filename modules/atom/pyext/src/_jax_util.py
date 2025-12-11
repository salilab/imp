import jax
import jax.numpy as jnp
import math
import functools
import IMP.atom

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


class _MDJaxInfo:
    def __init__(self, md):
        self._md = md
        self._sf = md.get_scoring_function().get_derived_object()
        ji = self._sf._get_jax()
        indexes = md.get_simulation_particle_indexes()
        deriv_func = jax.grad(ji.score_func)
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
            return X

        def apply_func(X):
            mass = X['mass'][indexes]
            # Get coordinates at t+(delta t) and velocities at t+(delta t/2)
            _propagate_coordinates(X, indexes, mass, time_step,
                                   velocity_cap)
            # Get new derivatives at t+(delta t)
            X["xyz'"] = deriv_func(X)["xyz"]
            # Get velocities at t+(delta t)
            _propagate_velocities(X, indexes, mass, time_step)
            return X

        self.init_func = init_func
        self.score_func = ji.score_func
        self.apply_func = apply_func

    def get_model_state(self):
        m = self._md.get_model()
        ji = self._sf._get_jax()
        X = ji.get_model_state()
        X['mass'] = m.get_floats_numpy(IMP.atom.Mass.get_mass_key())
        X['linvel'] = jax.numpy.array(
            m.get_vector3ds_numpy(IMP.atom.LinearVelocity.get_velocity_key()))
        X['xyz'] = jax.numpy.array(X['xyz'])
        return X


def _md_optimize(md, max_steps):
    # Get the number of steps that we can run in JAX, before having to
    # copy JAX arrays back to the IMP Model
    inner_steps = functools.reduce(
        math.gcd, [x.get_period() for x in md.optimizer_states], max_steps)
    n_loops = max_steps // inner_steps

    ji = md._get_jax()
    init_func = jax.jit(ji.init_func)
    score_func = jax.jit(ji.score_func)
    apply_func = jax.jit(
        lambda X: jax.lax.fori_loop(0, inner_steps,
                                    lambda i, X: ji.apply_func(X), X))

    X = init_func(ji.get_model_state())

    m = md.get_model()
    linvel = m.get_vector3ds_numpy(IMP.atom.LinearVelocity.get_velocity_key())
    xyz = m.get_spheres_numpy()[0]
    dxyz = m.get_sphere_derivatives_numpy()[0]

    n_step = 0
    for i in range(n_loops):
        X = apply_func(X)
        # Resync IMP Model arrays with JAX
        linvel[:] = X['linvel']
        xyz[:] = X['xyz']
        dxyz[:] = X["xyz'"]
        # Update any necessary OptimizerStates
        n_step += inner_steps
        for s in md.optimizer_states:
            if n_step % s.get_period() == 0:
                s.update_always()
    return score_func(X)

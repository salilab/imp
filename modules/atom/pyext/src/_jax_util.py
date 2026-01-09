import jax
import jax.numpy as jnp
import IMP.atom
from typing import NamedTuple
import IMP._jax_util


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
    # JAX random number key
    rkey: jax.Array
    # Any state used by OptimizerStates
    optimizer_states: dict
    # Indexes of all particles subject to MD
    simulation_indexes: jax.Array
    # Number of degrees of freedom in the system
    degrees_of_freedom: int
    # Time between integrator steps
    time_step: float

    def get_kinetic_energy(self):
        """Return the current kinetic energy of the system, in kcal/mol"""
        # Conversion factor to get energy in kcal/mol from velocities
        # in A/fs and mass in g/mol
        conversion = 1.0 / 4.1868e-4

        indexes = self.simulation_indexes
        velocity = self.X['linvel'][indexes]
        mass = self.X['mass'][indexes]
        return 0.5 * conversion * jnp.sum(
            mass * jnp.sum(jnp.square(velocity), axis=1))

    def get_kinetic_temperature(self, ekinetic):
        """Return the current kinetic temperature of the system"""
        # E = (n/2)kT  n=degrees of freedom, k = Boltzmann constant
        # Boltzmann constant, in kcal/mol
        boltzmann = 8.31441 / 4186.8
        return 2.0 * ekinetic / (self.degrees_of_freedom * boltzmann)


class _MDJaxInfo(IMP._jax_util.JaxOptimizerInfo):
    def __init__(self, md):
        super().__init__(md)
        deriv_func = jax.grad(self.score_func)
        velocity_cap = md.get_velocity_cap()
        # Would like to use math.isfinite here but it is not guaranteed
        # that a C++ "infinite" value is also considered to be math.inf
        if velocity_cap < 1e20:
            velocity_cap = jnp.array([velocity_cap] * 3)
        else:
            velocity_cap = None
        jax_optstates = [x.get_derived_object()._get_jax()
                         for x in md.optimizer_states]
        jax_optstates = [x for x in jax_optstates if x is not None]

        def init_func(X, key):
            X["xyz'"] = deriv_func(X)["xyz"]
            s = _MDState(
                X=X, steps=0, optimizer_states={}, rkey=key,
                simulation_indexes=md.get_simulation_particle_indexes(),
                degrees_of_freedom=md.get_degrees_of_freedom(),
                time_step=md.get_maximum_time_step())
            for js in jax_optstates:
                s = js.init_func(s)
            return s

        def apply_func(ms):
            X = ms.X
            indexes = ms.simulation_indexes
            steps = ms.steps + 1
            mass = X['mass'][indexes]
            # Make mass 2D so propagate functions can broadcast it over
            # the 2D coordinate/velocity arrays
            mass = mass.reshape(mass.shape[0], 1)
            # Get coordinates at t+(delta t) and velocities at t+(delta t/2)
            _propagate_coordinates(X, indexes, mass, ms.time_step,
                                   velocity_cap)
            # Get new derivatives at t+(delta t)
            X["xyz'"] = deriv_func(X)["xyz"]
            # Get velocities at t+(delta t)
            _propagate_velocities(X, indexes, mass, ms.time_step)
            ms = ms._replace(steps=steps)
            for js in jax_optstates:
                ms = jax.lax.cond(steps % js.period == 0, js.apply_func,
                                  lambda x: x, ms)
            return ms

        self.init_func = init_func
        self.apply_func = apply_func

        # Force MolecularDynamics to create linvel for all particles
        _ = md.get_simulation_particle_indexes()

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

    md_state = init_func(ji.get_model_state(),
                         key=IMP._jax_util.get_random_key())
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

import jax
import IMP.atom

# Conversion from derivatives (in kcal/mol/A) to acceleration (A/fs/fs)
_deriv_to_acceleration = -4.1868e-4


def _propagate_coordinates(X, indexes, mass, time_step):
    linvel = X['linvel'].at[indexes]
    dcoord = X["xyz'"][indexes]
    v = linvel.get() + time_step * 0.5 * dcoord * _deriv_to_acceleration / mass
    # todo: cap vel
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

        def init_func(X):
            X["xyz'"] = deriv_func(X)["xyz"]
            return X

        def apply_func(X):
            mass = X['mass'][indexes]
            # Get coordinates at t+(delta t) and velocities at t+(delta t/2)
            _propagate_coordinates(X, indexes, mass, time_step)
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

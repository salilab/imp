import jax
import jax.numpy as jnp
import functools
import IMP.atom
import jax.tree_util
from dataclasses import dataclass
import IMP._jax_util


# Conversion from derivatives (in kcal/mol/A) to acceleration (A/fs/fs)
_deriv_to_acceleration = -4.1868e-4


def _propagate_coordinates(jm, indexes, mass, time_step, velocity_cap=None):
    linvel = jm['linvel'].at[indexes]
    dcoord = jm["xyz'"][indexes]
    v = linvel.get() + time_step * 0.5 * dcoord * _deriv_to_acceleration / mass
    if velocity_cap is not None:
        v = jnp.clip(v, -velocity_cap, velocity_cap)
    jm['linvel'] = linvel.set(v)
    jm['xyz'] = jm['xyz'].at[indexes].add(v * time_step)


def _propagate_velocities(jm, indexes, mass, time_step):
    linvel = jm['linvel'].at[indexes]
    dcoord = jm["xyz'"][indexes]
    jm['linvel'] = linvel.add(
        time_step * 0.5 * dcoord * _deriv_to_acceleration / mass)


@jax.tree_util.register_dataclass
@dataclass
class _MolecularDynamics:
    """Track the state of a MolecularDynamics optimization using JAX"""

    # Current JAX Model
    jm: dict
    # Number of steps taken
    steps: int
    # JAX random number key
    rkey: jax.Array
    # Any persistent state used by OptimizerStates. Each OptimizerState's
    # _get_jax() method is given a unique index into this list.
    optimizer_states: list
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
        velocity = self.jm['linvel'][indexes]
        mass = self.jm['mass'][indexes]
        return 0.5 * conversion * jnp.sum(
            mass * jnp.sum(jnp.square(velocity), axis=1))

    def get_kinetic_temperature(self, ekinetic):
        """Return the current kinetic temperature of the system"""
        # E = (n/2)kT  n=degrees of freedom, k = Boltzmann constant
        # Boltzmann constant, in kcal/mol
        boltzmann = 8.31441 / 4186.8
        return 2.0 * ekinetic / (self.degrees_of_freedom * boltzmann)


class _MDJAXInfo(IMP._jax_util.JAXOptimizerInfo):
    def __init__(self, md):
        super().__init__(md)
        # score_func returns both score and a modified JAX Model, but
        # deriv_func only wants the first scalar argument (the score)
        deriv_func = jax.grad(lambda jm: self.score_func(jm)[0])
        velocity_cap = md.get_velocity_cap()
        # Would like to use math.isfinite here but it is not guaranteed
        # that a C++ "infinite" value is also considered to be math.inf
        if velocity_cap < 1e20:
            velocity_cap = jnp.array([velocity_cap] * 3)
        else:
            velocity_cap = None
        jax_optstates = self._setup_jax_optimizer_states()

        def init_func(jm, key):
            jm["xyz'"] = deriv_func(jm)["xyz"]
            s = _MolecularDynamics(
                jm=jm, steps=0, optimizer_states=[None] * len(jax_optstates),
                simulation_indexes=md.get_simulation_particle_indexes(),
                degrees_of_freedom=md.get_degrees_of_freedom(),
                rkey=key, time_step=md.get_maximum_time_step())
            for js in jax_optstates:
                s = js.init_func(s)
            return s

        def apply_func(ms):
            jm = ms.jm
            indexes = ms.simulation_indexes
            ms.steps += 1
            mass = jm['mass'][indexes]
            # Make mass 2D so propagate functions can broadcast it over
            # the 2D coordinate/velocity arrays
            mass = mass.reshape(mass.shape[0], 1)
            # Get coordinates at t+(delta t) and velocities at t+(delta t/2)
            _propagate_coordinates(jm, indexes, mass, ms.time_step,
                                   velocity_cap)
            # Get new derivatives at t+(delta t)
            jm["xyz'"] = deriv_func(jm)["xyz"]
            # Get velocities at t+(delta t)
            _propagate_velocities(jm, indexes, mass, ms.time_step)
            steps = ms.steps
            for js in jax_optstates:
                ms = jax.lax.cond(steps % js.period == 0, js.apply_func,
                                  lambda x: x, ms)
            return ms

        self.init_func = init_func
        self.apply_func = apply_func

        # Force MolecularDynamics to create linvel for all particles
        _ = md.get_simulation_particle_indexes()

    def get_jax_model(self):
        jm = super().get_jax_model()
        m = self._opt.get_model()
        jm['mass'] = m.get_floats_numpy(IMP.atom.Mass.get_mass_key())
        jm['linvel'] = jax.numpy.array(
            m.get_vector3ds_numpy(IMP.atom.LinearVelocity.get_velocity_key()))
        jm['xyz'] = jax.numpy.array(jm['xyz'])
        return jm


def _md_optimize(md, max_steps):
    from IMP.core._jax_util import _JAXOptimizer

    jopt = _JAXOptimizer(md, max_steps)
    inner_steps = jopt.inner_steps
    ji = md._get_jax()
    init_func = jax.jit(ji.init_func)
    score_func = jax.jit(ji.score_func)
    apply_func = jax.jit(
        lambda jm: jax.lax.fori_loop(0, inner_steps,
                                     lambda i, jm: ji.apply_func(jm), jm))

    md_state = init_func(ji.get_jax_model(),
                         key=IMP._jax_util.get_random_key())
    m = md.get_model()
    linvel = m.get_vector3ds_numpy(IMP.atom.LinearVelocity.get_velocity_key())
    xyz = m.get_spheres_numpy()[0]
    dxyz = m.get_sphere_derivatives_numpy()[0]

    for _ in jopt.loop():
        md_state = apply_func(md_state)
        # Resync IMP Model arrays with JAX
        jm = md_state.jm
        linvel[:] = jm['linvel']
        xyz[:] = jm['xyz']
        dxyz[:] = jm["xyz'"]
    score, md_state.jm = score_func(md_state.jm)
    return score


@jax.tree_util.register_dataclass
@dataclass
class _Bonds:
    """All information about chemical bonds"""

    # Ideal length of each bond
    length: jax.Array

    # Force constant per bond
    stiffness: jax.Array

    # Nx2 array of bonded particle indexes
    bonded_indexes: jax.Array


def _get_bonds(m, bond_indexes):
    """Given a list of particle indexes that are IMP.atom.Bond particles,
       return all data packed in a JAX _Bonds object"""
    bonded = []
    valid_indexes = []
    stiffness = []
    # todo: add utility functions to get these keys
    bond_length = m.get_numpy(IMP.FloatKey("bond length"))
    bond_stiffness = m.get_numpy(IMP.FloatKey("bond stiffness"))
    for b in bond_indexes:
        b = IMP.ParticleIndex(b)
        if not IMP.atom.Bond.get_is_setup(m, b):
            raise TypeError("%s is not a bond" % b)
        # Exclude bonds with negative length
        if bond_length[b] >= 0.0:
            valid_indexes.append(int(b))
            # Bonds with no or negative stiffness get default (1)
            if b >= len(bond_stiffness) or bond_stiffness[b] < 0:
                stiffness.append(1.0)
            else:
                stiffness.append(bond_stiffness[b])
            b = IMP.atom.Bond(m, b)
            bonded.append([b.get_bonded(i).get_particle_index()
                           for i in range(2)])
    return _Bonds(length=bond_length[valid_indexes],
                  stiffness=jnp.asarray(stiffness),
                  bonded_indexes=jnp.asarray(bonded))


@jax.tree_util.register_dataclass
@dataclass
class _Angles:
    """All information about chemical bond angles"""

    # Ideal value of each bond angle
    ideal: jax.Array

    # Force constant per angle
    stiffness: jax.Array

    # Nx3 array of bonded particle indexes
    bonded_indexes: jax.Array


def _get_angles(m, angle_indexes):
    """Given a list of particle indexes that are IMP.atom.Angle particles,
       return all data packed in a JAX _Angles object"""
    bonded = []
    valid_indexes = []
    ideal = m.get_numpy(IMP.atom.Angle.get_ideal_key())
    stiffness = m.get_numpy(IMP.atom.Angle.get_stiffness_key())
    for a in angle_indexes:
        a = IMP.ParticleIndex(a)
        if not IMP.atom.Angle.get_is_setup(m, a):
            raise TypeError("%s is not an angle" % a)
        # Exclude angles with negative stiffness
        if stiffness[a] > 0.0:
            valid_indexes.append(a)
            a = IMP.atom.Angle(m, a)
            bonded.append([a.get_particle(i).get_index() for i in range(3)])
    return _Angles(ideal=ideal[valid_indexes],
                   stiffness=stiffness[valid_indexes],
                   bonded_indexes=jnp.asarray(bonded))


@jax.tree_util.register_dataclass
@dataclass
class _Dihedrals:
    """All information about chemical bond dihedral angles"""

    # Ideal value of each dihedral
    ideal: jax.Array

    # Integer multiplicity per dihedral
    multiplicity: jax.Array

    # Force constant per dihedral
    stiffness: jax.Array

    # Nx4 array of bonded particle indexes
    bonded_indexes: jax.Array


def _get_dihedrals(m, angle_indexes):
    """Given a list of particle indexes that are IMP.atom.Dihedral particles,
       return all data packed in a JAX _Dihedrals object"""
    bonded = []
    valid_indexes = []
    ideal = m.get_numpy(IMP.atom.Dihedral.get_ideal_key())
    multiplicity = m.get_numpy(IMP.atom.Dihedral.get_multiplicity_key())
    stiffness = m.get_numpy(IMP.atom.Dihedral.get_stiffness_key())
    for a in angle_indexes:
        a = IMP.ParticleIndex(a)
        if not IMP.atom.Dihedral.get_is_setup(m, a):
            raise TypeError("%s is not a dihedral" % a)
        # Exclude angles with very small stiffness
        if abs(stiffness[a]) > 1e-6:
            valid_indexes.append(a)
            a = IMP.atom.Dihedral(m, a)
            bonded.append([a.get_particle(i).get_index() for i in range(4)])
    return _Dihedrals(ideal=ideal[valid_indexes],
                      multiplicity=multiplicity[valid_indexes],
                      stiffness=stiffness[valid_indexes],
                      bonded_indexes=jnp.asarray(bonded))


def _get_lennard_jones_score(lj, indexes):
    """Get a suitable JAX scoring function for the given LennardJones score"""
    def score(jm, aij, bij, repulsive_weight, attractive_weight,
              smoothing_function):
        xyzs = jm['xyz'][indexes]
        lj_types = jm['lennard_jones_type'][indexes]
        # Get index into aij/bij tables
        maxij = jnp.max(lj_types, axis=1)
        minij = jnp.min(lj_types, axis=1)
        lj_type_pair = (maxij+1)*maxij // 2 + minij
        dists = jnp.linalg.norm(xyzs[:, 0] - xyzs[:, 1], axis=1)
        A = aij[lj_type_pair] * repulsive_weight
        B = bij[lj_type_pair] * attractive_weight
        scores = A / dists**12 - B / dists**6
        return smoothing_function(scores, dists)

    sf = lj.get_smoothing_function().get_derived_object()
    # Function operates on a single distance + score; make it work on
    # an array instead using jax.vmap
    smoothing_function = jax.vmap(sf._get_jax())
    return functools.partial(
        score, aij=jnp.asarray(lj.get_repulsive_type_factors()),
        bij=jnp.asarray(lj.get_attractive_type_factors()),
        repulsive_weight=lj.get_repulsive_weight(),
        attractive_weight=lj.get_attractive_weight(),
        smoothing_function=smoothing_function)

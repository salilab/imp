import jax.random
import jax.numpy as jnp
import jax.tree_util
from dataclasses import dataclass
import IMP


def get_random_key():
    """Get a new JAX random key seeded from IMP's RNG"""
    return jax.random.key(IMP.random_number_generator())


@jax.tree_util.register_dataclass
@dataclass
class _RigidBodies:
    """Information on all rigid bodies in the Model"""

    # Internal coordinates indexed by particle index
    intcoord: jax.Array
    # Reference frame rotation quaternion indexed by rigid body index
    quaternion: jax.Array
    # Mapping from particle index to rigid body index
    rb_index_from_particle: dict
    # Mapping from rigid body index to particle index
    particle_from_rb_index: list


_RB_LIST_KEY = IMP.ModelKey("rigid body list")
_RB_QUAT_KEYS = [IMP.FloatKey("rigid_body_quaternion_%d" % i)
                 for i in range(4)]

def _get_rigid_bodies(m):
    assert m.get_has_data(_RB_LIST_KEY)
    rbl = m.get_data(_RB_LIST_KEY)
    rbl = IMP.SingletonContainer.get_from(rbl)
    particle_from_rb_index = rbl.get_contents()
    intcoord = m.get_internal_coordinates_numpy()
    quaternion = jnp.stack([m.get_numpy(rk)[particle_from_rb_index]
                            for rk in _RB_QUAT_KEYS], axis=1)
    return _RigidBodies(
        intcoord=intcoord, particle_from_rb_index=particle_from_rb_index,
        rb_index_from_particle={int(pi): rbi for (rbi, pi) in
                                enumerate(particle_from_rb_index)},
        quaternion=quaternion)


def _get_model_state(m, keys):
    """Convert an IMP Model object into a "model state" object suitable
       for use in JAX code. This is a simple dict. The dict keys are particle
       attribute names. Each dict value is a flat NumPy array of particle
       attribute values indexed by particle index. The model state dict will
       always include `xyz` and `r` items for particle XYZ coordinates and
       radii, but may include other attributes if they are used by restraints
       or optimizers. These arrays may be NumPy views of the IMP Model, or
       they may be copies (in which case if a JAX optimizer changes the values
       they may need to copied back to IMP).
       If the key "rigid_bodies" is given, information on all of the Model's
       rigid bodies is included."""
    xyz, r = m.get_spheres_numpy()
    X = {"xyz": xyz, "r": r}
    for k in keys:
        if k == 'rigid_bodies':
            X['rigid_bodies'] = _get_rigid_bodies(m)
        else:
            X[k.get_string()] = m.get_numpy(k)
    return X


def _get_score_constrained(m, score_func):
    """Given a JAX function that scores a model state, return a new function
       that first applies all ScoreStates (aka Constraints) and then returns
       both the score and the new model state."""
    # get all ScoreStates in sorted order
    apply_funcs = [ss.get_derived_object()._get_jax().apply_func
                   for ss in m.get_ordered_score_states()]

    def score_constrained_func(X):
        for f in apply_funcs:
            X = f(X)
        return score_func(X), X

    return score_constrained_func


class JAXRestraintInfo:
    """Information about a JAX implementation of one or more Restraints.

       These objects are returned by Restraint._get_jax() (usually using
       the helper function Restraint._wrap_jax()) or by
       RestraintsScoringFunction._get_jax(), and can be used to
       evaluate the score of a single restraint, a RestraintSet,
       or a RestraintsScoringFunction using JAX.

       @param m The IMP::Model that score_func acts on
       @param score_func The JAX scoring function
       @param weight The multiplication factor on the restraint(s)
       @param keys If given, a list of particle attribute Keys that the
                   scoring function uses (other than xyz and r), such
                   as Bayesian nuisances."""
    def __init__(self, m, score_func, weight, keys=None):
        self.m = m
        if weight == 1.0:
            self.score_func = score_func
        else:
            self.score_func = lambda X: weight * score_func(X)
        self._keys = frozenset(keys or ())

    def get_model_state(self):
        """Get Model data as a tree of NumPy arrays, X"""
        return _get_model_state(self.m, self._keys)


class JAXScoreInfo:
    """Information about a JAX implementation of a single Score
       (e.g. PairScore).

       These objects are returned by PairScore._get_jax() (usually using
       the PairScore._wrap_jax() helper function), and can be used to
       evaluate the Score using JAX. Usually this is done by a Restraint
       (see JAXRestraintInfo).

       @param score_func The JAX scoring function
       @param keys If given, a list of particle attribute Keys that the
                   scoring function uses (other than xyz and r), such
                   as Bayesian nuisances."""
    def __init__(self, score_func, keys=None):
        self.score_func = score_func
        self._keys = frozenset(keys or ())

    def get_model_state(self, m):
        """Get Model data for the given Model as a tree of NumPy arrays, X"""
        return _get_model_state(m, self._keys)


class JAXScoreStateInfo:
    """Information about a JAX implementation of a single ScoreState.

       These objects are returned by ScoreState._get_jax() (usually using
       the helper function ScoreState._wrap_jax()), and can be used to
       modify the model state using JAX.

       @param m The IMP::Model that apply_func acts on
       @param apply_func A JAX function that, given the current model
                         state, returns a new model state.
       @param keys If given, a list of particle attribute Keys that the
                   ScoreState uses (other than xyz and r), such
                   as mass."""
    def __init__(self, m, apply_func, keys=None):
        self.m = m
        self.apply_func = apply_func
        self._keys = frozenset(keys or ())

    def get_model_state(self):
        """Get Model data as a tree of NumPy arrays, X"""
        return _get_model_state(self.m, self._keys)


class JAXModifierInfo:
    """Information about a JAX implementation of a modifier
       (e.g. SingletonModifier)"""
    def __init__(self, apply_func, keys=None):
        self.apply_func = apply_func
        self._keys = frozenset(keys or ())


class JAXOptimizerInfo:
    """Information about a JAX implementation of an Optimizer.

       These objects are returned by Optimizer._get_jax(), and can be used
       to scample a scoring function using JAX.

       These public members are available:

       `init_func`: a JAX function which, given a model state (see
           get_model_state), creates and returns an initial optimizer state.
           This may just be the model state, or may add scores and statistics
           used by the optimizer.
       `score_func`: a JAX function which, given the model state, applies
           any ScoreStates (aka constraints) and returns its score and a
           new model state.
       `apply_func`: a JAX function which, given an optimizer state, performs
           one step of optimization and returns a new optimizer state.
    """

    def __init__(self, optimizer):
        self._opt = optimizer
        self._sf = optimizer.get_scoring_function().get_derived_object()
        ji = self._sf._get_jax()
        self.score_func = _get_score_constrained(
            optimizer.get_model(), ji.score_func)
        # Subclasses will fill in init_func and apply_func

    def _setup_jax_optimizer_states(self):
        """Setup and return a list of the OptimizerStates that have
           a JAX implementation"""
        state_index = 0
        jax_optstates = []
        for s in self._opt.optimizer_states:
            j = s.get_derived_object()._get_jax(state_index)
            if j is not None:
                state_index += 1
                jax_optstates.append(j)
        return jax_optstates

    def get_model_state(self):
        """Get Model data as a tree of NumPy arrays, X"""
        # By default just return the ScoringFunction's model state
        # todo: add any keys used by ScoreStates
        ji = self._sf._get_jax()
        return ji.get_model_state()


class JAXOptimizerStateInfo:
    """Information about a JAX implementation of an OptimizerState.

       These objects can be returned by OptimizerState._get_jax().

       These public members are available:

       `init_func`: a JAX function which, given an optimizer state,
           returns a (possibly modified) optimizer state.
       `apply_func`: a JAX function which, given an optimizer state, does
           the JAX equivalent of do_update() and returns a new optimizer state.
    """
    def __init__(self, optstate, init_func, apply_func):
        self.period = optstate.get_period()
        self.init_func = init_func
        self.apply_func = apply_func

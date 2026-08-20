import jax.random
import jax.tree_util
import IMP


def get_random_key():
    """Get a new JAX random key seeded from IMP's RNG"""
    return jax.random.key(IMP.random_number_generator())


def _get_jax_model(m, keys):
    """Convert an IMP Model object into a corresponding JAX model object
       suitable for use in JAX code. This is a simple dict. The dict keys are
       particle attribute names. Each dict value is usually a flat NumPy array
       of particle attribute values indexed by particle index. The JAX model
       dict will always include `xyz` and `r` items for particle XYZ
       coordinates and radii, but may include other attributes if they are
       used by restraints or optimizers. These arrays may be NumPy views of
       the IMP Model, or they may be copies (in which case if a JAX optimizer
       or ScoreState changes the values they may need to copied back to IMP).
       If the key "rigid_bodies" is given, information on all of the Model's
       rigid bodies is also included."""
    xyz, r = m.get_spheres_numpy()
    jm = {"xyz": xyz, "r": r}
    for k in keys:
        if k == 'rigid_bodies':
            from IMP.core._jax_rigid import _get_rigid_bodies
            jm['rigid_bodies'] = _get_rigid_bodies(m)
        else:
            jm[k.get_string()] = m.get_numpy(k)
    return jm


def _get_score_constrained(m, score_func):
    """Given a JAX function that scores a JAX Model, return a new function
       that first applies all ScoreStates (aka Constraints) and then returns
       both the score and the new JAX Model."""
    # get all ScoreStates in sorted order
    apply_funcs = [ss.get_derived_object()._get_jax().apply_func
                   for ss in m.get_ordered_score_states()]

    def score_constrained_func(jm):
        for f in apply_funcs:
            jm = f(jm)
        return score_func(jm), jm

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
            self.score_func = lambda jm: weight * score_func(jm)
        self._keys = frozenset(keys or ())

    def get_jax_model(self):
        """Get Model data as a tree of NumPy arrays"""
        return _get_jax_model(self.m, self._keys)


class JAXScoreInfo:
    """Information about a JAX implementation of a single Score
       (e.g. PairScore).

       These objects are returned by PairScore._get_jax() (usually using
       the PairScore._wrap_jax() helper function), and can be used to
       evaluate the Score using JAX. Usually this is done by a Restraint
       (see JAXRestraintInfo).

       @param m The IMP::Model that score_func acts on
       @param score_func The JAX scoring function
       @param keys If given, a list of particle attribute Keys that the
                   scoring function uses (other than xyz and r), such
                   as Bayesian nuisances."""
    def __init__(self, m, score_func, keys=None):
        self.m = m
        self.score_func = score_func
        self._keys = frozenset(keys or ())

    def get_jax_model(self):
        """Get Model data as a tree of NumPy arrays"""
        return _get_jax_model(self.m, self._keys)


class JAXScoreStateInfo:
    """Information about a JAX implementation of a single ScoreState.

       These objects are returned by ScoreState._get_jax() (usually using
       the helper function ScoreState._wrap_jax()), and can be used to
       modify the JAX Model.

       @param m The IMP::Model that apply_func acts on
       @param apply_func A JAX function that, given the current JAX Model,
                         returns a new JAX Model.
       @param keys If given, a list of particle attribute Keys that the
                   ScoreState uses (other than xyz and r), such
                   as mass."""
    def __init__(self, m, apply_func, keys=None):
        self.m = m
        self.apply_func = apply_func
        self._keys = frozenset(keys or ())

    def get_jax_model(self):
        """Get Model data as a tree of NumPy arrays"""
        return _get_jax_model(self.m, self._keys)


class JAXModifierInfo:
    """Information about a JAX implementation of a modifier
       (e.g. SingletonModifier)"""
    def __init__(self, apply_func, keys=None):
        self.apply_func = apply_func
        self._keys = frozenset(keys or ())


class _NullScoringFunction:
    """Helper class for an optimizer that has had .set_scoring_function([])
       called on it"""
    def __init__(self, m):
        self.m = m

    def _get_jax(self):
        # Always return a score of zero, and reference the optimizer's model
        return JAXRestraintInfo(m=self.m, score_func=lambda jm: 0.0,
                                weight=1.0)


class JAXOptimizerInfo:
    """Information about a JAX implementation of an Optimizer.

       These objects are returned by Optimizer._get_jax(), and can be used
       to scample a scoring function using JAX.

       These public members are available:

       `init_func`: a JAX function which, given a JAX Model (see
           get_jax_model), creates and returns an initial JAX Optimizer
           object. This includes the JAX model, and may add scores,
           statistics, or any other persistent state used by the optimizer.
       `score_func`: a JAX function which, given the JAX Model, applies
           any ScoreStates (aka constraints) and returns its score and a
           new JAX Model.
       `apply_func`: a JAX function which, given a JAX Optimizer object,
           performs one step of optimization and returns a new JAX Optimizer
           object.
    """

    def __init__(self, optimizer):
        self._opt = optimizer
        self._sf = self._get_scoring_function(optimizer)
        self._keys = frozenset()
        ji = self._sf._get_jax()
        self.score_func = _get_score_constrained(
            optimizer.get_model(), ji.score_func)
        # Subclasses will fill in init_func and apply_func

    def _get_scoring_function(self, optimizer):
        sf = optimizer.get_scoring_function()
        # NullScoringFunction has no Python implementation (and even if it
        # did, it has no valid Model pointer) so use our own helper class
        # instead
        if (sf._get_director_object() is None
                and sf.get_type_name() == 'NullScoringFunction'
                and sf.get_version_info().get_module() == 'IMP'):
            return _NullScoringFunction(optimizer.get_model())
        else:
            return sf.get_derived_object()

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

    def _get_score_state_keys(self):
        """Get keys used by all ScoreStates"""
        m = self._opt.get_model()
        return frozenset(k for ss in m.get_ordered_score_states()
                         for k in ss.get_derived_object()._get_jax()._keys)

    def get_jax_model(self):
        """Get Model data as a tree of NumPy arrays"""
        # Add keys used by the scoring function and ScoreStates to those we
        # need ourselves
        ji = self._sf._get_jax()
        sskeys = self._get_score_state_keys()
        return _get_jax_model(ji.m, ji._keys | self._keys | sskeys)


class JAXOptimizerStateInfo:
    """Information about a JAX implementation of an OptimizerState.

       These objects can be returned by OptimizerState._get_jax().

       These public members are available:

       `init_func`: a JAX function which, given a JAX Optimizer object,
           stores any needed OptimizerState persistent state in the
           object, and then returns the JAX Optimizer.
       `apply_func`: a JAX function which, given a JAX Optimizer, does
           the JAX equivalent of do_update() and returns a new JAX Optimizer.
    """
    def __init__(self, optstate, init_func, apply_func):
        self.period = optstate.get_period()
        self.init_func = init_func
        self.apply_func = apply_func

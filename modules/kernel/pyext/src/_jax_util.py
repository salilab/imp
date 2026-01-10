import jax.random
import IMP


def get_random_key():
    """Get a new JAX random key seeded from IMP's RNG"""
    return jax.random.key(IMP.random_number_generator())


def _get_model_state(m, keys):
    """Convert an IMP Model object into a "model state" object suitable
       for use in JAX code. This is a simple dict. The dict keys are particle
       attribute names. Each dict value is a flat NumPy array of particle
       attribute values indexed by particle index. The model state dict will
       always include `xyz` and `r` items for particle XYZ coordinates and
       radii, but may include other attributes if they are used by restraints
       or optimizers. These arrays may be NumPy views of the IMP Model, or
       they be copies (in which case if a JAX optimizer changes the values
       they may need to copied back to IMP). Optimizers may add more keys
       to this dict to track their own state."""
    xyz, r = m.get_spheres_numpy()
    X = {"xyz": xyz, "r": r}
    for k in keys:
        X[k.get_string()] = m.get_numpy(k)
    return X


class JAXRestraintInfo:
    """Information about a JAX implementation of one or more Restraints.

       These objects are returned by Restraint._get_jax() (usually using
       the helper function Restraint_wrap_jax()) or by
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


class JAXOptimizerInfo:
    """Information about a JAX implementation of an Optimizer.

       These objects are returned by Optimizer._get_jax(), and can be used
       to scample a scoring function using JAX.

       These public members are available:

       `init_func`: a JAX function which, given a model state (see
           get_model_state), creates and returns an initial optimizer state.
           This may just be the model state, or may add scores and statistics
           used by the optimizer.
       `score_func`: a JAX function which, given the model state, returns
           its score.
       `apply_func`: a JAX function which, given an optimizer state, performs
           one step of optimization and returns a new optimizer state.
    """

    def __init__(self, optimizer):
        self._opt = optimizer
        self._sf = optimizer.get_scoring_function().get_derived_object()
        ji = self._sf._get_jax()
        self.score_func = ji.score_func
        # Subclasses will fill in init_func and apply_func

    def get_model_state(self):
        """Get Model data as a tree of NumPy arrays, X"""
        # By default just return the ScoringFunction's model state
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

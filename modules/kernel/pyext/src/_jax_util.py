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
    X = { "xyz": xyz, "r": r }
    for k in keys:
        X[k.get_string()] = m.get_numpy(k)
    return X


class JaxRestraintInfo:
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


class JaxScoreInfo:
    """Information about a JAX implementation of a single Score
       (e.g. PairScore).

       These objects are returned by PairScore._get_jax() (usually using
       the PairScore._wrap_jax() helper function), and can be used to
       evaluate the Score using JAX. Usually this is done by a Restraint
       (see JaxRestraintInfo).

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

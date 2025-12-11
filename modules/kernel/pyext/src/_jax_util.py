class JaxInfo:
    """Information about a JAX implementation of part of the scoring function.

       These objects are returned by Restraint._get_jax(),
       RestraintsScoringFunction._get_jax(), or PairScore._get_jax(), and
       can be used to evaluate the IMP scoring function using JAX.

       @param m The IMP::Model that this function acts on (can be None
                for PairScores)
       @param score_func The JAX scoring function
       @param weight The multiplication factor on the restraint,
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
        xyz, r = self.m.get_spheres_numpy()
        X = { "xyz": xyz, "r": r }
        for k in self._keys:
            X[k.get_string()] = self.m.get_numpy(k)
        return X

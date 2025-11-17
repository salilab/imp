class JaxInfo:
    """Information about a JAX implementation of part of the scoring function.

       These objects are returned by Restraint._get_jax(),
       RestraintsScoringFunction._get_jax(), or PairScore._get_jax(), and
       can be used to evaluate the IMP scoring function using JAX

       @param m The IMP::Model that this function acts on (can be None
                for PairScores)
       @param score_func The JAX scoring function
       @param weight The multiplication factor on the restraint"""
    def __init__(self, m, score_func, weight):
        self.m = m
        if weight == 1.0:
            self.score_func = score_func
        else:
            self.score_func = lambda X: weight * score_func(X)

    def get_model_state(self):
        """Get Model data as a tree of NumPy arrays, X"""
        xyz, r = self.m.get_spheres_numpy()
        # todo: add other arrays if needed by scores (e.g. R, nuisances)
        X = { "xyz": xyz }
        return X

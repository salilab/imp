%pythoncode %{

class PythonExamplePairScore(IMP.PairScore):
    """An example PairScore written in Python.
       This should be functionally equivalent to the C++ ExamplePairScore.
    """
    def __init__(self, x0, k):
        super().__init__()
        self.x0, self.k = x0, k

    def evaluate_index(self, m, pip, da):
        """Pure Python implementation of the score"""
        d0 = IMP.core.XYZ(m, pip[0])
        d1 = IMP.core.XYZ(m, pip[1])
        diff = (d0.get_coordinates()
                - d1.get_coordinates()).get_magnitude() - self.x0
        score = .5 * self.k * diff * diff
        if da:
            # derivatives are requested
            delta = d0.get_coordinates() - d1.get_coordinates()
            udelta = delta.get_unit_vector()
            dv = self.k * diff
            d0.add_to_derivatives(udelta * dv, da)
            d1.add_to_derivatives(-udelta * dv, da)
        return score

    def _get_jax(self):
        """Implementation of the score using JAX"""
        import jax.numpy as jnp
        import functools
        def pair_score(X, indexes, x0, k):
            xyzs = X['xyz'][indexes]
            diff = jnp.linalg.norm(xyzs[:,0] - xyzs[:,1], axis=1) - x0
            return 0.5 * k * diff * diff
        f = functools.partial(pair_score, x0=self.x0, k=self.k)
        return self._wrap_jax(f)

    def do_get_inputs(self, m, pis):
        return IMP.get_particles(m, pis)

%}

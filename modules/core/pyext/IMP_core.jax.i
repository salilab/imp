%extend IMP::core::HarmonicDistancePairScore {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def jax_harmonic_distance_pair_score(X, indexes, d, k):
            xyzs = X['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_harmonic_distance_pair_score,
                              d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::HarmonicSphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def jax_score(X, indexes, d, k):
            xyzs = X['xyz'][indexes]
            rs = X['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::RestraintsScoringFunction {
  %pythoncode %{
    def _get_jax(self):
        import IMP._jax_util
        jis = [r.get_derived_object()._get_jax() for r in self.restraints]
        funcs = [j.score_func for j in jis]
        keys = frozenset(x for j in jis for x in j._keys)
        def jax_sf(X):
            return sum(f(X) for f in funcs)
        return IMP._jax_util.JaxRestraintInfo(
            m=self.get_model(), score_func=jax_sf, weight=1.0, keys=keys)
  %}
}

%extend IMP::core::PairRestraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        score = self.get_score_object().get_derived_object()
        ji = score._get_jax()
        score_jax = ji.score_func
        indexes = jnp.array([self.get_index()])
        def jax_pair_restraint(X):
            return jnp.sum(score_jax(X, indexes))
        return self._wrap_jax(jax_pair_restraint, keys=ji._keys)
  %}
}

%extend IMP::core::MonteCarloMover {
  %pythoncode %{
    def _get_jax(self):
        """Return a JAX implementation of this mover.
           Implement this method in a MonteCarloMover subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that given the random number generator state and the current model
           state, returns a proposed new model state and the proposal ratio.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}

%extend IMP::core::BallMover {
  %pythoncode %{
    def _get_jax(self):
        from IMP.algebra._jax_util import get_random_vector_in_3d_sphere
        indexes = self.get_indexes()
        keys = frozenset(self.get_keys())
        if keys != frozenset(IMP.core.XYZ.get_xyz_keys()):
            raise NotImplementedError("Only works for XYZ")
        radius = self.get_radius()

        def propose_func(k, X):
            v = get_random_vector_in_3d_sphere(k, radius)
            newX = X.copy()
            newX['xyz'] = X['xyz'].at[indexes].add(v)
            return newX, 1.0
        return propose_func
  %}
}

%extend IMP::core::MonteCarlo {
  %pythoncode %{
    def _get_jax(self):
        from IMP.core._jax_util import _MCJaxInfo
        return _MCJaxInfo(self)

    def _optimize_jax(self, max_steps):
        import IMP.core._jax_util
        return IMP.core._jax_util._mc_optimize(self, max_steps)
  %}
}

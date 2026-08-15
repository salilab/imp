%pythonbegin %{
  import functools
%}

%extend IMP::isd::UniformPrior {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        indexes = jnp.array([self.get_index()])
        def score(jm, lb, ub, k):
            nuisance = jm['nuisance'][indexes]
            return 0.5 * k * (nuisance - jnp.clip(nuisance, lb, ub)) ** 2
        rng = self.get_range()
        f = functools.partial(score, lb=rng[0], ub=rng[1], k=self.get_k())
        return self._wrap_jax(f, keys=[Scale.get_scale_key()])
  %}
}

%extend IMP::isd::JeffreysRestraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        indexes = jnp.array([self.get_index()])
        def score(jm):
            nuisance = jm['nuisance'][indexes]
            return jnp.sum(jnp.log(nuisance))
        return self._wrap_jax(score, keys=[Scale.get_scale_key()])
  %}
}

%extend IMP::isd::LogWrapper {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        funcs, keys = self._get_restraint_jax_funcs_keys()
        def jax_sf(jm):
            scores = jnp.asarray([f(jm) for f in funcs])
            return -jnp.log(jnp.prod(scores))
        return self._wrap_jax(jax_sf, keys=keys)
  %}
}

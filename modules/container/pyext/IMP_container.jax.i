%extend IMP::container::ListPairContainer {
  %pythoncode %{
    def _get_static_contents(self):
        return self.get_contents()
  %}
}

%extend IMP::container::PairsRestraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        container = self.get_container().get_derived_object()
        score = self.get_score_object().get_derived_object()
        ji = score._get_jax()
        score_jax = ji.score_func
        indexes = container._get_static_contents()
        def jax_pairs_restraint(X):
            return jnp.sum(score_jax(X, indexes))
        return self._wrap_jax(jax_pairs_restraint, keys=ji._keys)
  %}
}

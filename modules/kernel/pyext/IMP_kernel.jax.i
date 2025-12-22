%extend IMP::Restraint {
  %pythoncode %{
  def _wrap_jax(self, score_func, keys=None):
      """Use in _get_jax() to wrap the JAX scoring function with other
         model- and restraint-specific information"""
      from IMP._jax_util import JaxInfo
      return JaxInfo(m=self.get_model(), score_func=score_func,
                     weight=self.get_weight(), keys=keys)
  %}
}

%extend IMP::internal::_ConstRestraint {
  %pythoncode %{
    def _get_jax(self):
        value = self.get_value()
        return self._wrap_jax(lambda X: value)
  %}
}

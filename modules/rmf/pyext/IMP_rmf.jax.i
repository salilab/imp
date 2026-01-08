%extend IMP::rmf::SaveOptimizerState {
  %pythoncode %{
    def _get_jax(self):
        """No JAX implementation, but this OptimizerState should work
           correctly if JAX arrays are copied back to IMP.Model"""
        return None
  %}
}

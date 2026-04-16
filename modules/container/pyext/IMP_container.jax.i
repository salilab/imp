%pythonbegin %{
  import warnings
%}

%extend IMP::container::ListSingletonContainer {
  %pythoncode %{
    def _get_static_contents(self):
        return self.get_contents()
  %}
}

%extend IMP::container::ListPairContainer {
  %pythoncode %{
    def _get_static_contents(self):
        return self.get_contents()
  %}
}

%extend IMP::container::AllPairContainer {
  %pythoncode %{
    def _get_static_contents(self):
        return self.get_contents()
  %}
}

%extend IMP::container::ClosePairContainer {
  %pythoncode %{
    def _get_static_contents(self):
        warnings.warn("No JAX support for close pairs, so using all "
                      "possible pairs; this may be slow.", IMP.JAXWarning)
        return self.get_range_indexes()
  %}
}

%extend IMP::container::CloseBipartitePairContainer {
  %pythoncode %{
    def _get_static_contents(self):
        warnings.warn("No JAX support for close pairs, so using all "
                      "possible pairs; this may be slow.", IMP.JAXWarning)
        return self.get_range_indexes()
  %}
}

%extend IMP::container::SingletonsRestraint {
  %pythoncode %{
    def _get_jax(self):
        from . import _jax_util
        return _jax_util._get_jax_container_restraint(self)
  %}
}

%extend IMP::container::PairsRestraint {
  %pythoncode %{
    def _get_jax(self):
        from . import _jax_util
        return _jax_util._get_jax_container_restraint(self)
  %}
}

%extend IMP::container::SingletonsConstraint {
  %pythoncode %{
    def _get_jax(self):
        from . import _jax_util
        return _jax_util._get_jax_container_constraint(self)
  %}
}

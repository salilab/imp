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

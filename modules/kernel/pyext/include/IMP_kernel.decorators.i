
%extend IMP::Decorator {
  bool __eq__(Decorator o) {
    return *self == o;
  }
  bool __eq__(Particle* o) {
    return *self == o;
  }
  bool __bool__() {
    return self->get_is_valid();
  }
  %pythoncode %{
    __nonzero__ = __bool__
  %}
}

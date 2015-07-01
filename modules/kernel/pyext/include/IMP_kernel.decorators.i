
%extend IMP::Decorator {
  bool __eq__(Decorator o) {
    return *self == o;
  }
  bool __eq__(Particle* o) {
    return *self == o;
  }
  bool __nonzero__() {
    return self->get_is_valid();
  }
}

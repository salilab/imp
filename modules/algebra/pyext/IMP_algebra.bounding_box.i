
/* Provide our own implementations for some operators */
%ignore IMP::algebra::BoundingBoxD::operator+=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") BoundingBoxD::__iadd__(const IMP::algebra::BoundingBoxD<D> &) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") BoundingBoxD::__iadd__(const IMP::algebra::VectorD<D> &) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") BoundingBoxD::__iadd__(double) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
 }
}

%extend IMP::algebra::BoundingBoxD {
  /* Make sure that default-constructed BoundingBoxKD objects cannot
     be made in Python */
  BoundingBoxD() {
    IMP_USAGE_CHECK(D > 0, "The constructor can not be used "
                           << "with a variable dim bounding box.");
    return new IMP::algebra::BoundingBoxD<D>();
  }

  IMP::algebra::VectorD<D> __getitem__(unsigned int index) const {
    if (index >= 2) throw IMP::base::IndexException("");
    return self->get_corner(index);
  }
  /*void __setitem__(unsigned int index, double val) {
    self->operator[](index) = val;
  }*/
  /* Ignore C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const IMP::algebra::BoundingBoxD<D> &o) { self->operator+=(o); }
  void __iadd__(const IMP::algebra::VectorD<D> &o) { self->operator+=(o); }
  void __iadd__(double o) { self->operator+=(o); }
  unsigned int __len__() {return 2;}
  /* Provide support for operator+ */
  const IMP::algebra::BoundingBoxD<D> & __add__(const IMP::algebra::BoundingBoxD<D> &o) { return self->operator+(o); }
  const IMP::algebra::BoundingBoxD<D> & __add__(const IMP::algebra::VectorD<D> &o) { return self->operator+(o); }
  const IMP::algebra::BoundingBoxD<D> & __add__(double o) { return self->operator+(o); }
};
%ignore IMP::algebra::BoundingBoxD::BoundingBoxD();

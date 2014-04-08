
%define IMP_ALGEBRA_VECTOR(D)
/* Provide our own implementations for some operators */
%ignore IMP::algebra::VectorD<D>::operator[];
%ignore IMP::algebra::VectorD<D>::operator+=;
%ignore IMP::algebra::VectorD<D>::operator*=;
%ignore IMP::algebra::VectorD<D>::operator/=;
%ignore IMP::algebra::VectorD<D>::operator-=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") VectorD<D>::__iadd__(const IMP::algebra::VectorD<D> &) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") VectorD<D>::__imul__(double) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") VectorD<D>::__idiv__(double) %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") VectorD<D>::__isub__(const IMP::algebra::VectorD<D> &) %{
    def __isub__(self, *args):
        $action(self, *args)
        return self
  %}
 }
}

%extend IMP::algebra::VectorD<D> {
  double __getitem__(unsigned int index) const {
    if (index >= D) throw IMP::base::IndexException("");
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, double val) {
    self->operator[](index) = val;
  }
  /* Ignore C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const IMP::algebra::VectorD<D> &o) { self->operator+=(o); }
  void __imul__(double f) { self->operator*=(f); }
  void __idiv__(double f) { self->operator/=(f); }
  void __isub__(const IMP::algebra::VectorD<D> &o) { self->operator-=(o); }
  unsigned int __len__() { return self->get_dimension(); }
  const IMP::algebra::VectorD<D> __rmul__(double f) const {return self->operator*(f);}
  std::string __str__() const {std::ostringstream oss; self->show(oss); return oss.str();}
  std::string __repr__() const {std::ostringstream oss; self->show(oss); return oss.str();}
  int __cmp__(const IMP::algebra::VectorD<D> &) const {
    IMP_UNUSED(self);
    IMP_THROW("Geometric primitives cannot be compared",
              IMP::base::ValueException);
  }
};
%enddef

IMP_ALGEBRA_VECTOR(-1);
IMP_ALGEBRA_VECTOR(1);
IMP_ALGEBRA_VECTOR(2);
IMP_ALGEBRA_VECTOR(3);
IMP_ALGEBRA_VECTOR(4);
IMP_ALGEBRA_VECTOR(5);
IMP_ALGEBRA_VECTOR(6);
IMP_SWIG_ALGEBRA_VALUE_D(IMP::algebra,  Vector);

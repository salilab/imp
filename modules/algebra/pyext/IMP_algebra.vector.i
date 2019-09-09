
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
  double __getitem__(int index) const {
    if (index >= 0 && index < D) {
      return self->operator[](index);
    } else if (index <= -1 && index >= -(D)) {
      return self->operator[](index + D);
    } else {
      throw IMP::IndexException("VectorD index out of range");
    }
  }
  void __setitem__(int index, double val) {
    if (index >= 0 && index < D) {
      self->operator[](index) = val;
    } else if (index <= -1 && index >= -(D)) {
      self->operator[](index + D) = val;
    } else {
      throw IMP::IndexException("VectorD assignment index out of range");
    }
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
              IMP::ValueException);
  }
  bool __eq__(const IMP::algebra::VectorD<D> &) const {
    IMP_UNUSED(self);
    IMP_THROW("Geometric primitives cannot be compared",
              IMP::ValueException);
  }
  /* Support new-style "true" division */
  %pythoncode %{
  __truediv__ = __div__
  __itruediv__ = __idiv__
  %}
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

%define IMP_VECTOR(D)
/* Provide our own implementations for some operators */
%ignore IMP::VectorD<D>::operator[];
%ignore IMP::VectorD<D>::operator+=;
%ignore IMP::VectorD<D>::operator*=;
%ignore IMP::VectorD<D>::operator/=;
%ignore IMP::VectorD<D>::operator-=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
  %feature("shadow") VectorD<D>::__iadd__(const IMP::VectorD<D> &) %{
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
  %feature("shadow") VectorD<D>::__isub__(const IMP::VectorD<D> &) %{
    def __isub__(self, *args):
        $action(self, *args)
        return self
  %}
}

/* Add support for slicing to get/set of VectorD */
namespace IMP {
  %feature("shadow") VectorD<D>::__getitem__ %{
    def __getitem__(self, index):
        if isinstance(index, slice):
            return [self[i] for i in range(*index.indices(len(self)))]
        else:
            return $action(self, index)
  %}

  %feature("shadow") VectorD<D>::__setitem__ %{
    def __setitem__(self, index, val):
        if isinstance(index, slice):
            if not hasattr(val, '__iter__'):
                raise TypeError("must assign iterable to extended slice")
            inds = range(*index.indices(len(self)))
            if len(inds) != len(val):
                raise ValueError(
                    "attempt to assign sequence of size %d "
                    "to extended slice of size %d" % (len(val), len(inds)))
            for ind, v in zip(inds, val):
                self[ind] = v
        else:
            return $action(self, index, val)
  %}
}

%feature("python:maybecall", "0") IMP::VectorD<D>::__cmp__;
%feature("python:maybecall", "0") IMP::VectorD<D>::__eq__;
%extend IMP::VectorD<D> {
  /* Ignore C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const IMP::VectorD<D> &o) { self->operator+=(o); }
  void __imul__(double f) { self->operator*=(f); }
  void __idiv__(double f) { self->operator/=(f); }
  void __isub__(const IMP::VectorD<D> &o) { self->operator-=(o); }
  unsigned int __len__() { return self->get_dimension(); }
  const IMP::VectorD<D> __rmul__(double f) const {return self->operator*(f);}
  std::string __str__() const {std::ostringstream oss; self->show(oss); return oss.str();}
  std::string __repr__() const {std::ostringstream oss; self->show(oss); return oss.str();}
  int __cmp__(const IMP::VectorD<D> &) const {
    IMP_UNUSED(self);
    IMP_THROW("Geometric primitives cannot be compared",
              IMP::ValueException);
  }
  bool __eq__(const IMP::VectorD<D> &) const {
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

IMP_SWIG_VALUE_SERIALIZE_IMPL(IMP, VectorD<D>);
%enddef

%define IMP_FIXED_SIZE_VECTOR(D)
IMP_VECTOR(D);

%extend IMP::VectorD<D> {
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
}
%enddef

IMP_VECTOR(-1);
IMP_FIXED_SIZE_VECTOR(1);
IMP_FIXED_SIZE_VECTOR(2);
IMP_FIXED_SIZE_VECTOR(3);
IMP_FIXED_SIZE_VECTOR(4);
IMP_FIXED_SIZE_VECTOR(5);
IMP_FIXED_SIZE_VECTOR(6);
IMP_SWIG_VALUE_D(IMP, Vector);

%extend IMP::VectorD<-1> {
  double __getitem__(int index) const {
    int dim = self->get_dimension();
    if (index >= 0 && index < dim) {
      return self->operator[](index);
    } else if (index <= -1 && index >= -(dim)) {
      return self->operator[](index + dim);
    } else {
      throw IMP::IndexException("VectorD index out of range");
    }
  }
  void __setitem__(int index, double val) {
    int dim = self->get_dimension();
    if (index >= 0 && index < dim) {
      self->operator[](index) = val;
    } else if (index <= -1 && index >= -(dim)) {
      self->operator[](index + dim) = val;
    } else {
      throw IMP::IndexException("VectorD assignment index out of range");
    }
  }
}

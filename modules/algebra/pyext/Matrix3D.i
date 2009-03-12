%ignore IMP::algebra::Matrix3D::operator=;

/* Provide our own implementations for some operators */
%ignore IMP::algebra::Matrix3D::operator+=;
%ignore IMP::algebra::Matrix3D::operator-=;
%ignore IMP::algebra::Matrix3D::operator*=;
%ignore IMP::algebra::Matrix3D::operator/=;

// Matrix3D
%include "IMP/algebra/MultiArray.h"
%include "IMP/algebra/Matrix3D.h"

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") Matrix3D::__iadd__ %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Matrix3D::__imul__ %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Matrix3D::__idiv__ %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Matrix3D::__isub__ %{
    def __isub__(self, *args):
        $action(self, *args)
        return self
  %}
 }
}

// Extend Matrix3D with _internal_get and _internal_set
%extend IMP::algebra::Matrix3D<float> {
  float _internal_get(int i, int j, int k) const {
    return self->operator()(i, j, k);
  }
  void _internal_set(int i, int j, int k, float val) {
    self->operator()(i, j, k) = val;
  }

  /* Ignore any C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const Matrix3D<float> &x) { self->operator+=(x); }
  void __iadd__(float x) { self->operator+=(x); }
  void __imul__(const Matrix3D<float> &x) { self->operator*=(x); }
  void __imul__(float x) { self->operator*=(x); }
  void __idiv__(const Matrix3D<float> &x) { self->operator/=(x); }
  void __idiv__(float x) { self->operator/=(x); }
  void __isub__(const Matrix3D<float> &x) { self->operator-=(x); }
  void __isub__(float x) { self->operator-=(x); }

  // Compose the python calls to _internal_get and _internal_set
  %pythoncode {
    def __setitem__(self, indx, val):
        return self._internal_set(indx[0], indx[1], indx[2], val)
    def __getitem__(self, indx):
        return self._internal_get(indx[0], indx[1], indx[2])
  }
}


// Instantiating the templates for Python
namespace IMP {
 namespace algebra {
   %template(boostmultiarray3D) ::boost::multi_array<float,3>;
   %template(multiarray3D) ::IMP::algebra::MultiArray<float,3>;
   %template(matrix3D) ::IMP::algebra::Matrix3D<float>;
 }
}




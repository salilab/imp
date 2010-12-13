%ignore IMP::algebra::Matrix3D::operator=;

/* Provide our own implementations for some operators */
%ignore IMP::algebra::Matrix3D::operator+=;
%ignore IMP::algebra::Matrix3D::operator-=;
%ignore IMP::algebra::Matrix3D::operator*=;
%ignore IMP::algebra::Matrix3D::operator/=;

/* Ignore friend operators */
%ignore operator+(const T& X, const This& a1);
%ignore operator-(const T& X, const This& a1);
%ignore operator*(const T& X, const This& a1);
%ignore operator/(const T& X, const This& a1);

// Matrix3D
%include "IMP/algebra/MultiArray.h"
%include "IMP/algebra/Matrix3D.h"

%define SWIG_MATRIX3D(PYCLASS, TYPE)
/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") PYCLASS::__iadd__ %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") PYCLASS::__imul__ %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") PYCLASS::__idiv__ %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") PYCLASS::__isub__ %{
    def __isub__(self, *args):
        $action(self, *args)
        return self
  %}
 }
}

// Extend Matrix3D with _internal_get and _internal_set
%extend IMP::algebra::Matrix3D<TYPE> {
  TYPE _internal_get(int i, int j, int k) const {
    return self->operator()(i, j, k);
  }
  void _internal_set(int i, int j, int k, TYPE val) {
    self->operator()(i, j, k) = val;
  }

  /* Ignore any C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const Matrix3D<TYPE> &x) { self->operator+=(x); }
  void __iadd__(TYPE x) { self->operator+=(x); }
  void __imul__(const Matrix3D<TYPE> &x) { self->operator*=(x); }
  void __imul__(TYPE x) { self->operator*=(x); }
  void __idiv__(const Matrix3D<TYPE> &x) { self->operator/=(x); }
  void __idiv__(TYPE x) { self->operator/=(x); }
  void __isub__(const Matrix3D<TYPE> &x) { self->operator-=(x); }
  void __isub__(TYPE x) { self->operator-=(x); }

  // Compose the python calls to _internal_get and _internal_set
  %pythoncode {
    def __setitem__(self, indx, val):
        return self._internal_set(indx[0], indx[1], indx[2], val)
    def __getitem__(self, indx):
        return self._internal_get(indx[0], indx[1], indx[2])
  }
}
%enddef

SWIG_MATRIX3D(Matrix3D_f, float)
SWIG_MATRIX3D(Matrix3D, double)

// Instantiating the templates for Python
namespace IMP {
 namespace algebra {
   %template() ::boost::multi_array<float,3>;
   %template(_MultiArray3D_f) ::IMP::algebra::MultiArray<float,3>;
   %template(_MultiArray3D) ::IMP::algebra::MultiArray<double,3>;
   %template(_Matrix3D_f) ::IMP::algebra::Matrix3D<float>;
   %template(_Matrix3D) ::IMP::algebra::Matrix3D<double>;
 }
}

// Cannot instantiate Matrix3D directly, since SWIG complains about an
// attempt to redefine the template class; so rename it in the Python layer
%pythoncode {
  Matrix3D = _Matrix3D
  Matrix3D_f = _Matrix3D_f
  MultiArray3D = _MultiArray3D
  MultiArray3D_f = _MultiArray3D_f 
}

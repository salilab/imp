// Matrix2D
%include "IMP/algebra/MultiArray.h"
%include "IMP/algebra/Matrix2D.h"

// Extend Matrix2D with _internal_get and _internal_set
%extend IMP::algebra::Matrix2D<float> {
  float _internal_get(int i, int j) const {
    return self->operator()(i, j);
  }
  void _internal_set(int i, int j, float val) {
    self->operator()(i, j) = val;
  }
  IMP::algebra::Matrix2D<float> _internal_add(
                              const IMP::algebra::Matrix2D<float>& m) {
    return self->test_add(m);
  }
  IMP::algebra::Matrix2D<float> _internal_add(const float& x) {
    return self->test_add_f(x);
  }
  IMP::algebra::Matrix2D<float> _internal_sub(
                              const IMP::algebra::Matrix2D<float>& m) {
    return self->test_sub(m);
  }
  IMP::algebra::Matrix2D<float> _internal_sub(const float& x) {
    return self->test_sub_f(x);
  }
  IMP::algebra::Matrix2D<float> _internal_mul(
                              const IMP::algebra::Matrix2D<float>& m) {
    return self->test_mul(m);
  }
  IMP::algebra::Matrix2D<float> _internal_mul(const float& x) {
    return self->test_mul_f(x);
  }
  IMP::algebra::Matrix2D<float> _internal_div(
                              const IMP::algebra::Matrix2D<float>& m) {
    return self->test_div(m);
  }
  IMP::algebra::Matrix2D<float> _internal_div(const float &x) {
    return self->test_div_f(x);
  }

  // Compose the python calls to _internal_get and _internal_set
  %pythoncode {
    def __setitem__(self, indx, val):
      return self._internal_set(indx[0], indx[1], val)
    def __getitem__(self, indx):
      return self._internal_get(indx[0], indx[1])
    def __div__(self, m):
      return self._internal_div(m)
    def __mul__(self, m):
      return self._internal_mul(m)
    def __sub__(self, m):
      return self._internal_sub(m)
    def __add__(self, m):
      return self._internal_add(m)
  }
}


// Instantiating the templates for Python
namespace IMP {
 namespace algebra {
   %template(boostmultiarray2D) ::boost::multi_array<float,2>;
   %template(multiarray2D) ::IMP::algebra::MultiArray<float,2>;
   %template(matrix2D) ::IMP::algebra::Matrix2D<float>;
 }
}




%ignore IMP::algebra::Matrix2D::operator=;

// Matrix2D
%include "IMP/algebra/MultiArray.h"
%include "IMP/algebra/Matrix2D.h"

// Extend Matrix2D with _internal_get and _internal_set
%extend IMP::algebra::Matrix2D<float> {
  float _internal_get(int j, int i) const {
    return self->operator()(j,i);
  }
  void _internal_set(int j, int i, float val) {
    self->operator()(j,i) = val;
  }

  // Compose the python calls to _internal_get and _internal_set
  %pythoncode {
    def __setitem__(self, indx, val):
      self._internal_set(indx[0], indx[1], val)
    def __getitem__(self, indx):
      return self._internal_get(indx[0], indx[1])
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




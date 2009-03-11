%ignore IMP::algebra::Matrix3D::operator=;

// Matrix3D
%include "IMP/algebra/MultiArray.h"
%include "IMP/algebra/Matrix3D.h"

// Extend Matrix3D with _internal_get and _internal_set
%extend IMP::algebra::Matrix3D<float> {
  float _internal_get(int i, int j, int k) const {
    return self->operator()(i, j, k);
  }
  void _internal_set(int i, int j, int k, float val) {
    self->operator()(i, j, k) = val;
  }


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




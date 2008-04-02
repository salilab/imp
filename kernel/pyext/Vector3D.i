/* Provide our own implementations for operator[] */
%ignore IMP::Vector3D::operator[];

%extend IMP::Vector3D {
  float __getitem__(unsigned int index) const {
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, float val) {
    self->operator[](index) = val;
  }
};

%include "IMP/Vector3D.h"

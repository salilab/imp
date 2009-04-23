
/* Provide our own implementations for some operators */
%ignore IMP::algebra::SphericalCoords::operator[];

%extend IMP::algebra::SphericalCoords {
  double __getitem__(unsigned int index) const {
    if (index >= 3) throw IMP::IndexException("");
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, double val) {
    self->operator[](index) = val;
  }
};

%include "IMP/algebra/SphericalCoords.h"


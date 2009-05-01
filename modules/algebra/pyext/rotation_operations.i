%import "modules/algebra/pyext/algebra.i"

/* Provide our own implementations for some operators */
%ignore IMP::algebra::EulerAnglesZYZ::operator[];

%extend IMP::algebra::EulerAnglesZYZ {
  double __getitem__(unsigned int index) const {
    if (index >= 3) throw IMP::IndexException("");
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, double val) {
    self->operator[](index) = val;
  }
};


%import "IMP/algebra/rotation_operations.h"

// Instantiating the templates for Python
namespace IMP {
 namespace algebra {
   %template(rotate_matrix_2D) ::IMP::algebra::rotate_matrix_2D<float>;
   %template(rotate_matrix_2Dd) ::IMP::algebra::rotate_matrix_2D<double>;
   %template(auto_rotate_matrix_2D)
      ::IMP::algebra::auto_rotate_matrix_2D<float>;
   %template(auto_rotate_matrix_2Dd)
      ::IMP::algebra::auto_rotate_matrix_2D<double>;
 }
}

%include "IMP/algebra/rotation_operations.h"


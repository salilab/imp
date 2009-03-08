// Algebra
%module(directors="1") "IMP.algebra"

%{
#include "IMP.h"
#include "IMP/algebra.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "algebra_config.i"
%include "exception.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"

namespace IMP {
  namespace algebra {
    typedef VectorD<3> Vector3D;
  }
}

namespace boost {
template <class T, int D> class multi_array{};

namespace multi_array_types {
  typedef size_t size_type;
  typedef size_t difference_type;
  struct index;
  struct index_range{};
  struct extent_range{};
  struct index_gen{};
  struct extent_gen{};
}
}

/* Wrap our own base classes */
%include "VectorD.i"
%include "IMP/algebra/Rotation3D.h"
%include "IMP/algebra/Transformation3D.h"
%include "IMP/algebra/geometric_alignment.h"
%include "IMP/algebra/eigen_analysis.h"
%include "IMP/algebra/Plane3D.h"
%include "IMP/algebra/Cylinder3D.h"
%include "IMP/algebra/Sphere3D.h"
%include "IMP/algebra/Sphere3DPatch.h"
%include "IMP/algebra/Cone3D.h"
%include "IMP/algebra/vector_generators.h"
%include "IMP/algebra/io.h"
%include "Matrix2D.i"
%include "Matrix3D.i"

namespace IMP {
 namespace algebra {
   %template(random_vector_on_sphere) random_vector_on_sphere<3>;
   %template(random_vector_in_sphere) random_vector_in_sphere<3>;
   %template(random_vector_in_box) random_vector_in_box<3>;
   %template(random_vector_on_unit_sphere) random_vector_on_unit_sphere<3>;
   %template(random_vector_in_unit_sphere) random_vector_in_unit_sphere<3>;
   %template(random_vector_in_unit_box) random_vector_in_unit_box<3>;
   %template(random_vector_on_box) random_vector_on_box<3>;
 }
}

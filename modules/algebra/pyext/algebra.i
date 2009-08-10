// Algebra
%module(directors="1") "IMP.algebra"

%{
#include "IMP.h"
#include "IMP/algebra.h"
%}

%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"
%include "IMP_typemaps.i"

%include "exception.i"
%include "std_vector.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "IMP_keys.i"

%include "IMP/algebra/config.h"

/* Ignore friends */
%ignore compose(const Rotation3D &a, const Rotation3D &b);

namespace IMP {
  namespace algebra {
    typedef BoundingBoxD<3> BoundingBox3D;
    typedef VectorD<3> Vector3D;
    typedef VectorD<4> Vector4D;

    /* Copy returned references to non-refcounted classes */
    %apply REFCOPY & { const Rotation2D & };
    %apply REFCOPY & { const Rotation3D & };
    %apply REFCOPY & { const Vector3D & };
    %apply REFCOPY & { const Vector4D & };
    %apply REFCOPY & { const BoundingBox3D & };
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

/* Wrap our own classes */
%include "VectorD.i"
%include "BoundingBoxD.i"
%include "Transformation3D.i"
%include "Matrix2D.i"
%include "Matrix3D.i"
%include "SphericalCoords.i"
%include "IMP/algebra/geometric_alignment.h"
%include "IMP/algebra/eigen_analysis.h"
%include "IMP/algebra/Segment3D.h"
%include "IMP/algebra/Plane3D.h"
%include "IMP/algebra/Cylinder3D.h"
%include "IMP/algebra/Sphere3D.h"
%include "IMP/algebra/Sphere3DPatch.h"
%include "IMP/algebra/Cone3D.h"
%include "IMP/algebra/vector_generators.h"
%include "IMP/algebra/io.h"
%include "IMP/algebra/endian.h"
%include "IMP/algebra/Rotation2D.h"
%include "IMP/algebra/shortest_segment.h"

namespace IMP {
 namespace algebra {
   %template(random_vector_on_sphere) random_vector_on_sphere<3>;
   %template(random_vector_in_sphere) random_vector_in_sphere<3>;
   %template(random_vector_in_box) random_vector_in_box<3>;
   %template(random_vector_on_unit_sphere) random_vector_on_unit_sphere<3>;
   %template(random_vector_in_unit_sphere) random_vector_in_unit_sphere<3>;
   %template(random_vector_in_unit_box) random_vector_in_unit_box<3>;
   %template(random_vector_on_box) random_vector_on_box<3>;
   %template(uniform_cover_sphere) uniform_cover_sphere<3>;
   %template(basis_vector) basis_vector<3>;
   %template(zeros) zeros<3>;
   // for debugging
   %template(Sphere3Ds) ::std::vector<IMP::algebra::Sphere3D>;
   %template(SpherePair) ::std::pair<IMP::algebra::Sphere3D,IMP::algebra::Sphere3D>;
   %template(Sphere3DPairs) ::std::vector< SpherePair >;
   // rotation operations
   %template(Rotation3Ds) ::std::vector<Rotation3D>;
 }
}

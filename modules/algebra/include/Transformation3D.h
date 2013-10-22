/**
 *  \file IMP/algebra/Transformation3D.h
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_TRANSFORMATION_3D_H
#define IMPALGEBRA_TRANSFORMATION_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "Rotation3D.h"
#include "BoundingBoxD.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class Transformation3D;
Transformation3D compose(const Transformation3D &a, const Transformation3D &b);
#endif

//! Simple 3D transformation class
/** The rotation is applied first, and then the point is translated.
    \see IMP::core::Transform
    \geometry
*/
class IMPALGEBRAEXPORT Transformation3D : public GeometricPrimitiveD<3> {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(Transformation3D);
  //! construct an invalid transformation
  Transformation3D() {}
  /** basic constructor*/
  Transformation3D(const Rotation3D &r, const Vector3D &t = Vector3D(0, 0, 0))
      : trans_(t), rot_(r) {}
  /** Construct a transformation with an identity rotation.*/
  Transformation3D(const Vector3D &t)
      : trans_(t), rot_(get_identity_rotation_3d()) {}
  ~Transformation3D();
  //! transform
  Vector3D get_transformed(const Vector3D &o) const {
    return rot_.get_rotated(o) + trans_;
  }
  //! apply transformation (rotate and then translate)
  Vector3D operator*(const Vector3D &v) const { return get_transformed(v); }
  /** compose two rigid transformation such that for any vector v
      (rt1*rt2)*v = rt1*(rt2*v) */
  Transformation3D operator*(const Transformation3D &tr) const {
    return compose(*this, tr);
  }
  const Transformation3D &operator*=(const Transformation3D &o) {
    *this = compose(*this, o);
    return *this;
  }
  /** Compute the transformation which, when composed with b, gives *this.
      That is a(x)== d(b(x)) for all x.

      For consistency, this should probably have a nice name, but
      I don't know what name to give it.
  */
  Transformation3D operator/(const Transformation3D &b) const {
    Transformation3D ret = compose(*this, b.get_inverse());
    return ret;
  }
  const Transformation3D &operator/=(const Transformation3D &o) {
    *this = *this / o;
    return *this;
  }
  const Rotation3D &get_rotation() const { return rot_; }
  const Vector3D &get_translation() const { return trans_; }

  IMP_SHOWABLE_INLINE(Transformation3D, {
    rot_.show(out);
    out << " || " << trans_;
  });
  Transformation3D get_inverse() const;

 private:
  Vector3D trans_;  // translation
  Rotation3D rot_;  // rotation
};

IMP_VALUES(Transformation3D, Transformation3Ds);

//! Return a transformation that does not do anything
/** See Transformation3D */
inline Transformation3D get_identity_transformation_3d() {
  return Transformation3D(get_identity_rotation_3d(), Vector3D(0.0, 0.0, 0.0));
}

//! Generate a Transformation3D object from a rotation around a point
/** Rotate about a point rather than the origin.
  \param[in] point Center to rotate about
  \param[in] rotation The rotation to perform

  See Transformation3D
*/
inline Transformation3D get_rotation_about_point(const Vector3D &point,
                                                 const Rotation3D &rotation) {
  return Transformation3D(rotation, (rotation * (-point) + point));
}

//! compose two transformations
/** For any vector v (a*b)*v = a*(b*v).
    See Transformation3D
 */
inline Transformation3D compose(const Transformation3D &a,
                                const Transformation3D &b) {
  return Transformation3D(compose(a.get_rotation(), b.get_rotation()),
                          a.get_transformed(b.get_translation()));
}

class Transformation2D;

//! Builds a 3D transformation from a 2D one.
/**
  \note The 3D transformation is built with the 2D rotation becoming a rotation
  around the z axis.
  **/
IMPALGEBRAEXPORT Transformation3D
    get_transformation_3d(const Transformation2D &t2d);

//! Get a local transformation
/**
  \note randomly select an axis that passes to the input point
  and rotate around it
  \param[in] origin the origin of the rotation
  \param[in] max_translation detault value is 5
  \param[in] max_angle_in_rad default value is 15 degree in radians
  **/
IMPALGEBRAEXPORT Transformation3D
    get_random_local_transformation(Vector3D origin,
                                    double max_translation = 5.,
                                    double max_angle_in_rad = 0.26);

//! Return a bounding box containing the transformed box
inline BoundingBoxD<3> get_transformed(const BoundingBoxD<3> &bb,
                                       const Transformation3D &tr) {
  BoundingBoxD<3> nbb;
  for (unsigned int i = 0; i < 2; ++i) {
    for (unsigned int j = 0; j < 2; ++j) {
      for (unsigned int k = 0; k < 2; ++k) {
        algebra::Vector3D v(bb.get_corner(i)[0], bb.get_corner(j)[1],
                            bb.get_corner(k)[2]);
        nbb += tr.get_transformed(v);
      }
    }
  }
  return nbb;
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_TRANSFORMATION_3D_H */

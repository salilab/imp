/**
 *  \file IMP/algebra/Transformation2D.h
 *  \brief 2D transformations.
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
**/

#ifndef IMPALGEBRA_TRANSFORMATION_2D_H
#define IMPALGEBRA_TRANSFORMATION_2D_H

#include <IMP/algebra/algebra_config.h>

#include "Vector2D.h"
#include "Rotation2D.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
// #ifndef IMP_DOXYGEN
class Transformation2D;
Transformation2D compose(const Transformation2D &a, const Transformation2D &b);
#endif

//! Simple 2D transformation class
/** \geometry
*/
class IMPALGEBRAEXPORT Transformation2D : public GeometricPrimitiveD<2> {
 public:

  //! constructor. An invalid transformation is built
  Transformation2D() {}

  //! basic constructor from a Rotation2D and translation vector
  Transformation2D(const Rotation2D &r, const Vector2D &t = Vector2D(0.0, 0.0))
      : trans_(t), rot_(r) {}

  //! Constructor for a transformation with an identity rotation.
  Transformation2D(const Vector2D &t)
      : trans_(t), rot_(get_identity_rotation_2d()) {}

  ~Transformation2D();

  //! Perform the transformation on an 2D vector
  /**
    \param[in] o vector where the transformation is applied
    \note: The transformation is done firstly applying the rotation and then
    the translation
  **/
  Vector2D get_transformed(const Vector2D &o) const {
    return rot_.get_rotated(o) + trans_;
  }

  //! Perform the transformation on an 2D vector
  /**
    \note: The transformation is done firstly applying the rotation and then
    the translation
  **/
  Vector2D operator*(const Vector2D &v) const { return get_transformed(v); }

  //! compose two transformations
  /**
    \ note The transformations are composed such that for any vector v
      (rt1*rt2)*v = rt1*(rt2*v)
  **/
  Transformation2D operator*(const Transformation2D &tr) const {
    return compose(*this, tr);
  }

  //! See help for operator*
  const Transformation2D &operator*=(const Transformation2D &o) {
    *this = compose(*this, o);
    return *this;
  }

  //! Compute the transformation d which, when composed with b, gives this one.
  //! That is a(x)== d(b(x)) for all x.
  Transformation2D operator/(const Transformation2D &b) const {
    Transformation2D ret = compose(*this, b.get_inverse());
    return ret;
  }

  //! See help for operator/
  const Transformation2D &operator/=(const Transformation2D &o) {
    *this = *this / o;
    return *this;
  }

  //! Returns the rotation
  const Rotation2D get_rotation() const { return rot_; }

  void set_rotation(double angle) { rot_.set_angle(angle); }

  //! Returns the translation
  const Vector2D get_translation() const { return trans_; }

  //! Sets the translation
  void set_translation(const Vector2D &v) {
    trans_[0] = v[0];
    trans_[1] = v[1];
  }

  IMP_SHOWABLE_INLINE(Transformation2D, {
    rot_.show(out);
    out << " || " << trans_;
  });

  //! Returns the inverse transformation
  Transformation2D get_inverse() const;

 private:
  Vector2D trans_;  //tranlation
  Rotation2D rot_;  //rotation
};

IMP_VALUES(Transformation2D, Transformation2Ds);

//! Returns a transformation that does not do anything
/**
  See Transformation2D
**/
inline Transformation2D get_identity_transformation_2d() {
  return Transformation2D(get_identity_rotation_2d(), Vector2D(0.0, 0.0));
}

//! Generates a Transformation2D object from a rotation around a point
/**
  Generates a Transformation2D to rotate about a point rather than the origin.
  \param[in] point Center to rotate about
  \param[in] rotation The rotation to perform (defined taking the origin as
  reference, not the new point).
  See Transformation2D
*/
inline Transformation2D get_rotation_about_point(const Vector2D &point,
                                                 const Rotation2D &rotation) {
  return Transformation2D(rotation, (rotation.get_rotated(-point) + point));
}

//! compose two transformations
/** For any vector v (a*b)*v = a*(b*v).
    See Transformation2D
 */
inline Transformation2D compose(const Transformation2D &a,
                                const Transformation2D &b) {
  Rotation2D R = compose(a.get_rotation(), b.get_rotation());
  return Transformation2D(R, a.get_transformed(b.get_translation()));
}

IMPALGEBRA_END_NAMESPACE
#endif /* IMPALGEBRA_TRANSFORMATION_2D_H */

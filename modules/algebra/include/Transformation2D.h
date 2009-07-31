/**
 *  \file Transformation2D.h
 *  \brief 2D transformations.
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
**/

#ifndef IMPALGEBRA_TRANSFORMATION_2D_H
#define IMPALGEBRA_TRANSFORMATION_2D_H

#include "config.h"

#include "VectorD.h"
#include "Matrix2D.h"
#include "Rotation2D.h"

IMPALGEBRA_BEGIN_NAMESPACE

#ifndef IMP_DOXYGEN
class Transformation2D;
Transformation2D compose(const Transformation2D &a,
                         const Transformation2D &b);
#endif

//! Simple 2D transformation class
/** \see IMP::core::Transform
*/
class IMPALGEBRAEXPORT Transformation2D: public UninitializedDefault
{
public:
  //! public for swig
  IMP_NO_DOXYGEN(typedef Transformation2D This;)

  //! constructor. An invalid transformation is built
  Transformation2D(){}

  //! basic constructor from a Rotation2D and translation vector
  Transformation2D(const Rotation2D& r,
                   const VectorD<2>& t=VectorD<2>(0.0,0.0)):
    trans_(t), rot_(r){}

  //! Constructor for a transformation with an identity rotation.
  Transformation2D(const VectorD<2>& t):
    trans_(t), rot_(identity_rotation2D()){}

  ~Transformation2D();

  //! Perform the transformation on an 2D vector
  /**
    \param[in] o vector where the transformation is applied
    \note: The transformation is done firstly applying the rotation and then
    the translation
  **/
  VectorD<2> transform(const VectorD<2> &o) const {
    return rot_.rotate(o) + trans_;
  }

  //! Perform the transformation on an 2D vector
  /**
    \note: The transformation is done firstly applying the rotation and then
    the translation
  **/
  VectorD<2> operator*(const VectorD<2> &v) const {
    return transform(v);
  }

  //! compose two transformations
  /**
    \ note The transformations are composed such that for any vector v
      (rt1*rt2)*v = rt1*(rt2*v)
  **/
  Transformation2D operator*(const Transformation2D &tr) const {
    return compose(*this, tr);
  }

  //! See help for operator*
  const Transformation2D& operator*=(const Transformation2D &o) {
    *this=compose(*this, o);
    return *this;
  }

  //! Compute the transformation d which, when composed with b, gives this one.
  //! That is a(x)== d(b(x)) for all x.
  Transformation2D operator/(const Transformation2D &b) const {
    Transformation2D ret= compose(*this, b.get_inverse());
    return ret;
  }

  //! See help for operator/
  const Transformation2D& operator/=(const Transformation2D &o) {
    *this= *this/o;
    return *this;
  }

  //! Returns the rotation
  const Rotation2D& get_rotation() const {return rot_;}

  void set_rotation(double angle) {
    rot_.set_angle(angle);
  }

  //! Returns the translation
  const VectorD<2>& get_translation()const{return trans_;}

  //! Sets the translation
  template<typename T1>
  void set_translation(T1& v) {
    trans_[0]=v[0];
    trans_[1]=v[1];
  }

  //! Prints the transformation
  void show(std::ostream& out = std::cout) const {
    rot_.show(out);
    out << " || " << trans_;
  }

  //! Returns the inverse transformation
  Transformation2D get_inverse() const;

private:
  VectorD<2> trans_; //tranlation
  Rotation2D rot_;  //rotation
};


IMP_OUTPUT_OPERATOR(Transformation2D)

//! Returns a transformation that does not do anything
/**
  \relatesalso Transformation2D
**/
inline Transformation2D identity_transformation2D() {
  return Transformation2D(identity_rotation2D(),VectorD<2>(0.0,0.0));
};

//! Generates a Transformation2D object from a rotation around a point
/**
  Generates a Transformation2D to rotate about a point rather than the origin.
  \param[in] point Center to rotate about
  \param[in] rotation The rotation to perform (defined taking the origin as
  reference, not the new point).
  \relatesalso Transformation2D
*/
inline Transformation2D rotation_about_point(const VectorD<2> &point,
                     const Rotation2D &rotation) {
  return Transformation2D(rotation, (rotation.rotate(-point)+point));
};

//! compose two transformations
  /** For any vector v (a*b)*v = a*(b*v).
      \relatesalso Transformation2D
   */
inline Transformation2D compose(const Transformation2D &a,
                                const Transformation2D &b){
  return Transformation2D(compose(a.get_rotation(),
                                  b.get_rotation()).get_rotation(),
                          a.transform(b.get_translation()));
};

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_TRANSFORMATION_2D_H */

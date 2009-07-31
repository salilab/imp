/**
 *  \file Rotation2D.h
 *  \brief Classes and operations related with rotations
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_ROTATION_2D_H
#define IMPALGEBRA_ROTATION_2D_H

#include "config.h"
#include "utility.h"
#include "VectorD.h"
#include "Matrix2D.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Stores a 2D rotation matrix
/**
  \note This class requires the angles to be given in radians, and the
  convention used is that the rotations are performed rotating counterclockwise
  (right hand side convention).
**/
class Rotation2D: public UninitializedDefault
{
public:
  Rotation2D(): angle_(std::numeric_limits<double>::quiet_NaN()) {};

  //! Builds the matrix for the given angle
  Rotation2D(double angle) {
    set_angle(angle);
  }

  //! rotates a 2D point
  /**
  * \param[in] o a 2D vector to be rotated
  */
  VectorD<2> rotate(const VectorD<2> &o) const {
    IMP_assert(!is_nan(angle_),
               "Attempting to use uninitialized rotation");
    return rotate(o[0],o[1]);
  }

  //! rotates a 2D point
  VectorD<2> rotate(const double x,const double y) const {
    IMP_assert(!is_nan(angle_),
               "Attempting to use uninitialized rotation");
    return VectorD<2>(c_*x-s_*y , s_*x+c_*y);
  }

  //! Returns the matrix for the inverse rotation
  Rotation2D get_inverse() const {
    IMP_assert(!is_nan(angle_),
               "Attempting to use uninitialized rotation");
    return Rotation2D(-angle_);
  }

  //! sets the angle for the rotation
  /**
  * \param[in] angle the angle
  */
  void set_angle(double angle) {
    angle_ = angle;
    c_ = cos(angle);
    s_ = sin(angle);
  }

  //! gets the angle
  double get_angle() const {
    return angle_;
  }

  //! Prints the angle
  void show(std::ostream& out = std::cout, std::string delim=" ") const {
    out << "Rotation2D (radians): " << angle_;
  }

private:
  double angle_; // angle
  double c_; // cosine of the angle
  double s_; // sine of the angle
};


//! Builds an identity rotation in 2D
inline Rotation2D identity_rotation2D() {
  return Rotation2D(0.0);
};

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_ROTATION_2D_H */

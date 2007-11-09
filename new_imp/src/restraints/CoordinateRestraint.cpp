/**
 *  \file CoordinateRestraint.cpp \brief Absolute position restraint.
 *
 *  Optimize based on distance from an absolute position.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/CoordinateRestraint.h"

namespace IMP
{

//! Constructor - set up the values and indexes for this coordinate restraint.
/** \param[in] model Pointer to the model.
    \param[in] p1 Pointer to particle of the restraint.
    \param[in] axis String indicating the axis of absolute reference:
                    X_AXIS, Y_AXIS, Z_AXIS, XY_RADIAL, XZ_RADIAL, YZ_RADIAL,
                    XYZ_SPHERE
    \param[in] score_func_params Parameters for creating a score function.
 */
CoordinateRestraint::CoordinateRestraint(Model* model, Particle* p1,
    const std::string axis, BasicScoreFuncParams* score_func_params)
{
  add_particle(p1);
  x_ = FloatKey("x");
  y_ = FloatKey("y");
  z_ = FloatKey("z");

  axis_ = axis;
  score_func_ = score_func_params->create_score_func();
}

//! Destructor
CoordinateRestraint::~CoordinateRestraint()
{
}


//! Calculate the score for this coordinate restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float CoordinateRestraint::evaluate(DerivativeAccumulator *accum)
{
  Float score = 0.0, deriv;
  Float x, y, z;
  Float dx = 0.0;
  Float dy = 0.0;
  Float dz = 0.0;
  Float current_distance;

  // get current position of particle
  x = get_particle(0)->get_value(x_);
  y = get_particle(0)->get_value(y_);
  z = get_particle(0)->get_value(z_);

  // restrain the x coordinate
  if (axis_ == "X_AXIS") {
    score = (*score_func_)(x, dx);
  }

  // restrain the y coordinate
  else if (axis_ == "Y_AXIS") {
    score = (*score_func_)(y, dy);
  }

  // restrain the z coordinate
  else if (axis_ == "Z_AXIS") {
    score = (*score_func_)(z, dz);
  }

  // restrain the xy radial distance
  else if (axis_ == "XY_RADIAL") {
    current_distance = sqrt(x * x + y * y);
    score = (*score_func_)(sqrt(x * x + y * y), deriv);
    dx = x / current_distance * deriv;
    dy = y / current_distance * deriv;
  }

  // restrain the xz radial distance
  else if (axis_ == "XZ_RADIAL") {
    current_distance = sqrt(x * x + z * z);
    score = (*score_func_)(current_distance, deriv);
    dx = x / current_distance * deriv;
    dz = z / current_distance * deriv;
  }

  // restrain the yz radial distance
  else if (axis_ == "YZ_RADIAL") {
    current_distance = sqrt(y * y + z * z);
    score = (*score_func_)(current_distance, deriv);
    dy = y / current_distance * deriv;
    dz = z / current_distance * deriv;
  }

  // restrain the xyz spherical distance (Euclidian distance from origin)
  else if (axis_ == "XYZ_SPHERE") {
    current_distance = sqrt(x * x + y * y + z * z);
    score = (*score_func_)(current_distance, deriv);
    dx = x / current_distance * deriv;
    dy = y / current_distance * deriv;
    dz = z / current_distance * deriv;
  }

  // if needed, use the partial derivatives
  if (accum) {
    get_particle(0)->add_to_derivative(x_, dx, *accum);
    get_particle(0)->add_to_derivative(y_, dy, *accum);
    get_particle(0)->add_to_derivative(z_, dz, *accum);
  }

  IMP_LOG(VERBOSE, axis_ << " score: " << score << "  x: " << x << " y: " << y
          << " z: " << z << "   dx: " << dx << " dy: " << dy << " dz: " << dz
          << std::endl);

  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void CoordinateRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "coordinate restraint (active):" << std::endl;
  } else {
    out << "coordinate restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: "
      << last_modified_by() << std::endl;
  out << "  particles: " << get_particle(0)->get_index() 
      << " and " << get_particle(0)->get_index();

  out << "  axis:" << axis_;
}

}  // namespace IMP

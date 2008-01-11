/**
 *  \file TorusRestraint.cpp \brief Score based on distance from torus interior
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/TorusRestraint.h"

namespace IMP
{

TorusRestraint::TorusRestraint(Particle* p1, const Float main_radius,
                               const Float tube_radius,
                               UnaryFunction* score_func)
{
  add_particle(p1);
  x_ = FloatKey("x");
  y_ = FloatKey("y");
  z_ = FloatKey("z");

  main_radius_ = main_radius;
  tube_radius_ = tube_radius;
  score_func_ = score_func;
}


//! Destructor
TorusRestraint::~TorusRestraint()
{
  delete score_func_;
}


//! Calculate the score for this restraint for the current model state.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float TorusRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(score_func_);

  Float tube_center_x, tube_center_y;
  Float xy_distance_from_center;
  Float distance_from_tube_center;
  Float torus_feature;
  Float score, deriv;
  Float x, y, z;
  Float dx, dy, dz;

  IMP_LOG(VERBOSE, "... evaluating torus restraint.");
  // get current position of particle
  x = get_particle(0)->get_value(x_);
  y = get_particle(0)->get_value(y_);
  z = get_particle(0)->get_value(z_);

  // get the x, y distance from the origin
  xy_distance_from_center = sqrt(x * x + y * y);
  if (xy_distance_from_center == 0.0) {
    // if no direction is favored,
    // ... pick some arbitrary direction to head towards the
    // ... torus interior
    xy_distance_from_center = 0.001;
    x = 0.001;
  }

  // calculate the cross-section_center (large radius away from origin with z=0)
  tube_center_x = x * main_radius_ / xy_distance_from_center;
  tube_center_y = y * main_radius_ / xy_distance_from_center;
  // by definition: tube_center_z = 0;

  // translate to cross-section center
  x -= tube_center_x;
  y -= tube_center_y;

  // calculate the distance from the cross-section_center
  distance_from_tube_center = sqrt(x * x + y * y + z * z);

  // subtract the small radius of the torus so that if the point is within the
  // ... torus the value is negative
  torus_feature = distance_from_tube_center - tube_radius_;

  // if in the torus, partials should be zero
  if (torus_feature <= 0) {
    score = 0.0;
    deriv = 0.0;
  }

  // otherwise, derivatives should reduce feature as torus is neared
  // the derivative vector stays constant independent of distance from torus
  // i.e. a unit vector
  else {
    score = (*score_func_)(torus_feature, deriv);
  }

  // if needed, use the partial derivatives
  if (accum) {
    dx = deriv * x / distance_from_tube_center;
    dy = deriv * y / distance_from_tube_center;
    dz = deriv * z / distance_from_tube_center;

    get_particle(0)->add_to_derivative(x_, dx, *accum);
    get_particle(0)->add_to_derivative(y_, dy, *accum);
    get_particle(0)->add_to_derivative(z_, dz, *accum);
  }

  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void TorusRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "torus restraint (active):" << std::endl;
  } else {
    out << "torus restraint (inactive):" << std::endl;
  }

  out << "  version: " << version() << "  ";
  out << "  last_modified_by: " << last_modified_by() << std::endl;
  out << "  particle: " << get_particle(0)->get_index() << "  ";

  out << "  main radius:" << main_radius_;
  out << "  tube radius:" << tube_radius_;
}

}  // namespace IMP

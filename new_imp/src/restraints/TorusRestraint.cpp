/**
 *  \file TorusRestraint.cpp \brief Score based on distance from torus interior
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/TorusRestraint.h"

namespace IMP
{

//! Constructor - set up the values and indexes for this torus restraint.
/** Expects coordinates to be labeled "x", "y", and "z" in the particles.

    \param[in] model Pointer to the model.
    \param[in] p1 Pointer to particle of the restraint.
    \param[in] main_radius The main radius from the origin to the midline
                           of the tube.
    \param[in] tube_radius The tube radius is min distance from the tube
                           midline to the tube surface.
    \param[in] score_func_params Parameters for creating a score function.
 */
TorusRestraint::TorusRestraint(Model& model, Particle* p1,
                               const Float main_radius, const Float tube_radius,
                               BasicScoreFuncParams* score_func_params)
{
  model_data_ = model.get_model_data();

  particles_.push_back(p1);
  x1_ = p1->get_float_index(std::string("x"));
  y1_ = p1->get_float_index(std::string("y"));
  z1_ = p1->get_float_index(std::string("z"));

  main_radius_ = main_radius;
  tube_radius_ = tube_radius;
  score_func_ = score_func_params->create_score_func();
}


//! Destructor
TorusRestraint::~TorusRestraint()
{
}


//! Calculate the score for this restraint for the current model state.
/** \param[in] calc_deriv If true, partial first derivatives should
                          be calculated.
    \return Current score.
 */
Float TorusRestraint::evaluate(bool calc_deriv)
{
  Float tube_center_x, tube_center_y;
  Float xy_distance_from_center;
  Float distance_from_tube_center;
  Float torus_feature;
  Float score, deriv;
  Float x, y, z;
  Float dx, dy, dz;

  IMP_LOG(VERBOSE, "... evaluating torus restraint.");
  // get current position of particle
  x = model_data_->get_float(x1_);
  y = model_data_->get_float(y1_);
  z = model_data_->get_float(z1_);

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
  if (calc_deriv) {
    dx = deriv * x / distance_from_tube_center;
    dy = deriv * y / distance_from_tube_center;
    dz = deriv * z / distance_from_tube_center;

    model_data_->add_to_deriv(x1_, dx);
    model_data_->add_to_deriv(y1_, dy);
    model_data_->add_to_deriv(z1_, dz);
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

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  x1:" << model_data_->get_float(x1_);
  out << "  y1:" << model_data_->get_float(y1_);
  out << "  z1:" << model_data_->get_float(z1_) << std::endl;

  out << "  dx1:" << model_data_->get_deriv(x1_);
  out << "  dy1:" << model_data_->get_deriv(y1_);
  out << "  dz1:" << model_data_->get_deriv(z1_) << std::endl;

  out << "  main radius:" << main_radius_;
  out << "  tube radius:" << tube_radius_;
}

}  // namespace IMP

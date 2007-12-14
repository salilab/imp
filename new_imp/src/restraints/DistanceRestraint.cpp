/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/restraints/DistanceRestraint.h"

namespace IMP
{

//! particles must be at least this far apart to calculate the restraint
const Float DistanceRestraint::MIN_DISTANCE = 0.0000001;

//! Constructor - set up the restraint using a given mean.
/** \param[in] model Pointer to the model.
    \param[in] p1 Pointer to first particle in distance restraint.
    \param[in] p2 Pointer to second particle in distance restraint.
    \param[in] score_func_params Score function parameters for the restraint.
 */
DistanceRestraint::DistanceRestraint(Model* model, Particle* p1, Particle* p2,
                                     BasicScoreFuncParams* score_func_params)
{
  set_up(model, p1, p2, score_func_params);
}

//! Constructor - set up the restraint using a given attribute.
/** \param[in] model Pointer to the model.
    \param[in] p1 Pointer to first particle in distance restraint.
    \param[in] p2 Pointer to second particle in distance restraint.
    \param[in] attr_name Name of attribute to be used to determine the
                         expected distance (e.g. "radius").
    \param[in] score_func_params Score function parameters for the restraint.
 */
DistanceRestraint::DistanceRestraint(Model* model, Particle* p1, Particle* p2,
                                     FloatKey attr_name,
                                     BasicScoreFuncParams* score_func_params)
{

  // LogMsg(VERBOSE, "Construct distance restraint: " << attr_name);
  Float mean = p1->get_value(attr_name)
               + p2->get_value(attr_name);

  score_func_params->set_mean(mean);
  set_up(model, p1, p2, score_func_params);
}


//! Do set up for the distant restraint constructors.
/** \param[in] model Pointer to the model.
    \param[in] p1 Pointer to first particle in distance restraint.
    \param[in] p2 Pointer to second particle in distance restraint.
    \param[in] score_func_params Score function parameters for the restraint.
 */
void DistanceRestraint::set_up(Model* , Particle* p1, Particle* p2,
                               BasicScoreFuncParams* score_func_params)
{
  // LogMsg(VERBOSE, "Set up distance restraint.");
  add_particle(p1);
  x_ = FloatKey("x");
  y_ = FloatKey("y");
  z_ = FloatKey("z");

  add_particle(p2);

  score_func_ = score_func_params->create_score_func();
}


//! Destructor
DistanceRestraint::~DistanceRestraint()
{
}


//! Calculate the score for this distance restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float DistanceRestraint::evaluate(DerivativeAccumulator *accum)
{
  Float distance;
  Float delta_x, delta_y, delta_z;
  Float score;

  // we need deltas for calculating the distance and the derivatives
  delta_x = get_particle(0)->get_value(x_) - get_particle(1)->get_value(x_);
  delta_y = get_particle(0)->get_value(y_) - get_particle(1)->get_value(y_);
  delta_z = get_particle(0)->get_value(z_) - get_particle(1)->get_value(z_);

  // calculate the distance feature
  distance = sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  if (accum) {
    Float dx, dy, dz;
    Float deriv;

    // calculate the score and feature derivative based on the distance feature
    score = (*score_func_)(distance, deriv);

    if (distance >= DistanceRestraint::MIN_DISTANCE) {
      dx = delta_x / distance * deriv;
      dy = delta_y / distance * deriv;
      dz = delta_z / distance * deriv;
    } else {
      // avoid division by zero
      dx = dy = dz = 0.;
    }

    get_particle(0)->add_to_derivative(x_, dx, *accum);
    get_particle(1)->add_to_derivative(x_, -dx, *accum);

    get_particle(0)->add_to_derivative(y_, dy, *accum);
    get_particle(1)->add_to_derivative(y_, -dy, *accum);

    get_particle(0)->add_to_derivative(z_, dz, *accum);
    get_particle(1)->add_to_derivative(z_, -dz, *accum);
  }

  else {
    // calculate the score based on the distance feature
    score = (*score_func_)(distance);
  }

  IMP_LOG(VERBOSE, "distance: " << distance << "   score: " << score);
  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void DistanceRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "distance restraint (active):" << std::endl;
  } else {
    out << "distance restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  ";
  out << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  particles: " << get_particle(0)->get_index();
  out << " and " << get_particle(1)->get_index();

  out << "  mean:" << mean_;
  out << "  sd:" << sd_ << std::endl;
}

}  // namespace IMP

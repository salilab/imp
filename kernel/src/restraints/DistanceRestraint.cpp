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
#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{

//! particles must be at least this far apart to calculate the restraint
static const Float MIN_DISTANCE = 0.0000001;

DistanceRestraint::DistanceRestraint(Particle* p1, Particle* p2,
                                     ScoreFunc* score_func)
{
  add_particle(p1);
  add_particle(p2);

  score_func_ = score_func;
}


//! Destructor
DistanceRestraint::~DistanceRestraint()
{
  delete score_func_;
}


//! Calculate the score for this distance restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float DistanceRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(score_func_);

  Float d2=0, delta[3];
  Float score;

  XYZDecorator d0= XYZDecorator::cast(get_particle(0));
  XYZDecorator d1= XYZDecorator::cast(get_particle(1));
  for (int i=0; i< 3; ++i ){
    delta[i]= d0.get_coordinate(i)- d1.get_coordinate(i);
    d2+= square(delta[i]);
  }

  Float distance = std::sqrt(d2);

  // if needed, calculate the partial derivatives of the scores with respect
  // to the particle attributes
  // avoid division by zero if the distance is too small
  if (accum && distance >= MIN_DISTANCE) {
    Float deriv;

    // calculate the score and feature derivative based on the distance feature
    score = (*score_func_)(distance, deriv);

    for (int i=0; i< 3; ++i ){
      Float d= delta[i]/distance*deriv;
      d0.add_to_coordinate_derivative(i, d, *accum);
      d1.add_to_coordinate_derivative(i, -d, *accum);
    }

  }

  else {
    // calculate the score based on the distance feature
    score = (*score_func_)(distance);
  }

  IMP_LOG(VERBOSE, "For " << get_particle(0)->get_index() 
          << " and " << get_particle(1)->get_index() 
          << " distance: " << distance 
          << " score: " << score << std::endl);
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

  out << "  version: " << version() << "  ";
  out << "  last_modified_by: " << last_modified_by() << std::endl;
  out << "  particles: " << get_particle(0)->get_index();
  out << " and " << get_particle(1)->get_index();
  out << "  ";
  score_func_->show(out);
  out << std::endl;
}

}  // namespace IMP

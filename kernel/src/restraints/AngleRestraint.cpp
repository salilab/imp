/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/AngleRestraint.h"
#include "IMP/triplet_scores/AngleTripletScore.h"

namespace IMP
{

AngleRestraint::AngleRestraint(Particle* p1, Particle* p2, Particle* p3,
                               UnaryFunction* score_func)
{
  add_particle(p1);
  add_particle(p2);
  add_particle(p3);

  sf_= std::auto_ptr<AngleTripletScore>(new AngleTripletScore(score_func));
}

//! Calculate the score for this angle restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float AngleRestraint::evaluate(DerivativeAccumulator *accum)
{
  return sf_->evaluate(get_particle(0),
                       get_particle(1),
                       get_particle(2),
                       accum);
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void AngleRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "angle restraint (active):" << std::endl;
  } else {
    out << "angle restraint (inactive):" << std::endl;
  }

  out << "  version: " << version() << "  ";
  out << "  last_modified_by: " << last_modified_by() << std::endl;
  out << "  particles: " << get_particle(0)->get_index();
  out << ", " << get_particle(1)->get_index();
  out << " and " << get_particle(2)->get_index();
  out << "  ";
  sf_->show(out);
  out << std::endl;
}

}  // namespace IMP

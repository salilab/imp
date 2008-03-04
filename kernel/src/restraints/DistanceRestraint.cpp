/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/restraints/DistanceRestraint.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/pair_scores/DistancePairScore.h"

namespace IMP
{

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     Particle* p1, Particle* p2) :
    dp_(score_func)
{
  add_particle(p1);
  add_particle(p2);
}

//! Calculate the score for this distance restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float DistanceRestraint::evaluate(DerivativeAccumulator *accum)
{
  return dp_.evaluate(get_particle(0), get_particle(1), accum);
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

  get_version_info().show(out);
  out << "  particles: " << get_particle(0)->get_index();
  out << " and " << get_particle(1)->get_index();
  out << "  ";
  dp_.show(out);
  out << std::endl;
}

}  // namespace IMP

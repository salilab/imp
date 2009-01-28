/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZDecorator.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

IMPCORE_BEGIN_NAMESPACE

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     Particle* p1, Particle* p2) :
    dp_(score_func)
{
  p_[0]=p1;
  p_[1]=p2;
}

//! Calculate the score for this distance restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float DistanceRestraint::evaluate(DerivativeAccumulator *accum)
{
  return dp_.evaluate(p_[0], p_[1], accum);
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
  out << "  particles: " << p_[0]->get_name();
  out << " and " << p_[1]->get_name();
  out << "  ";
  dp_.show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE

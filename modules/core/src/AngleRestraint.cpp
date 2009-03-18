/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/AngleRestraint.h>
#include <IMP/core/AngleTripletScore.h>

IMPCORE_BEGIN_NAMESPACE

AngleRestraint::AngleRestraint(UnaryFunction* score_func,
                               Particle* p1, Particle* p2, Particle* p3)
{
  p_[0]=p1;
  p_[1]=p2;
  p_[2]=p3;

  sf_= new AngleTripletScore(score_func);
}

Float AngleRestraint::evaluate(DerivativeAccumulator *accum)
{
  return sf_->evaluate(p_[0], p_[1], p_[2],
                       accum);
}


void AngleRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "angle restraint (active):" << std::endl;
  } else {
    out << "angle restraint (inactive):" << std::endl;
  }

  get_version_info().show(out);
  out << "  particles: " << p_[0]->get_name();
  out << ", " << p_[1]->get_name();
  out << " and " << p_[2]->get_name();
  out << "  ";
  sf_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE

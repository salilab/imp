/**
 *  \file MyRestraint.cpp \brief Restraint between two particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/modulename/MyRestraint.h>
#include <IMP/log.h>

IMPMODULENAME_BEGIN_NAMESPACE

MyRestraint::MyRestraint(PairScore* score_func,
                         Particle* p1, Particle* p2) :
    ps_(score_func)
{
  add_particle(p1);
  add_particle(p2);
}

Float MyRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_LOG(VERBOSE, "Evaluating MyRestraint on "
          << get_particle(0)->get_index()
          << " and "
          << get_particle(1)->get_index() << std::endl);
  return ps_->evaluate(get_particle(0), get_particle(1), accum);
}


void MyRestraint::show(std::ostream& out) const
{
  out << "MyRestraint on "
      << get_particle(0)->get_index()
      << " and "
      << get_particle(1)->get_index()
      << " using " << *ps_ << std::endl;
}

IMPMODULENAME_END_NAMESPACE

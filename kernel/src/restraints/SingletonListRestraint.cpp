/**
 *  \file SingletonListRestraint.cpp 
 *  \brief Apply a score function to a list of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/SingletonListRestraint.h"
#include "IMP/SingletonScore.h"
#include "IMP/log.h"

#include <cmath>

namespace IMP
{

SingletonListRestraint::SingletonListRestraint(SingletonScore *s,
                                               const Particles &ps):
  ss_(s)
{
  add_particles(ps);
}


Float SingletonListRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(ss_.get());

  Float score=0;

  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    score += ss_->evaluate(get_particle(i), accum);
  }

  return score;
}


void SingletonListRestraint::show(std::ostream& out) const
{
  out << "List restraint with score function ";
  ss_->show(out);
  out << std::endl;
}

} // namespace IMP

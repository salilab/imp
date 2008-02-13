/**
 *  \file ListRestraint.cpp 
 *  \brief Apply a score fgunction toa list of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/SingletonScore.h"
#include "IMP/log.h"
#include "IMP/restraints/ListRestraint.h"

namespace IMP
{

ListRestraint::ListRestraint(const Particles &ps,
                             SingletonScore *s) : ss_(s)
{
  set_particles(ps);
}


Float ListRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(ss_.get());

  Float score=0;

  for (unsigned int i=0; i< number_of_particles(); ++i) {
    score += ss_->evaluate(get_particle(i), accum);
  }

  return score;
}


void ListRestraint::show(std::ostream& out) const
{
  out << "List restraint with score function ";
  ss_->show(out);
  out << std::endl;
}

} // namespace IMP

/**
 *  \file ExampleRestraint.cpp \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/examples/ExampleRestraint.h>
#include <IMP/log.h>

IMPEXAMPLES_BEGIN_NAMESPACE

ExampleRestraint::ExampleRestraint(PairScore* score_func,
                                   PairContainer *pc) : pc_(pc),
                                          f_(score_func) {}

Float ExampleRestraint::evaluate(DerivativeAccumulator *accum)
{
  Float score=0;
  for (PairContainer::ParticlePairIterator
       it= pc_->particle_pairs_begin();
       it != pc_->particle_pairs_end(); ++it) {
    score += f_->evaluate(it->first, it->second, accum);
  }

  return score;
}

/* Return a list of interacting sets. In this case it is pairs
   of particles so all the sets are of size 2. */
ParticlesList ExampleRestraint::get_interacting_particles() const
{
  ParticlesList ret;
  for (PairContainer::ParticlePairIterator it
       = pc_->particle_pairs_begin();
       it != pc_->particle_pairs_end(); ++it) {
    Particles s;
    s.push_back(it->first);
    s.push_back(it->second);
    ret.push_back(s);
  }
  return ret;
}

void ExampleRestraint::show(std::ostream& out) const
{
  out << "PairsRestraint with score function ";
  f_->show(out);
  out << " and container ";
  pc_->show(out);
  out << std::endl;
}

IMPEXAMPLES_END_NAMESPACE

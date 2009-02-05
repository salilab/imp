/**
 *  \file PairListRestraint.cpp
 *  \brief Apply a score function to a list of pairs of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/PairListRestraint.h>

#include <IMP/PairScore.h>
#include <IMP/log.h>
#include <IMP/deprecation.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

PairListRestraint::PairListRestraint(PairScore *s,
                                     const ParticlePairs &ps) : ss_(s)
{
  add_particle_pairs(ps);
  IMP_LOG(SILENT, "PairListRestraint is now deprecated. "
          << "You should start shifting to ParticlePairsRestraint"
          << std::endl);
  IMP_DEPRECATED(PairListRestraint, PairsRestraint);
}

PairListRestraint::~PairListRestraint()
{
}

Float PairListRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(ss_.get());

  Float score=0;

  for (unsigned int i=0; i< get_number_of_particle_pairs(); ++i) {
    score += ss_->evaluate(get_particle_pair(i).first,
                           get_particle_pair(i).second, accum);
  }

  return score;
}


ParticlesList PairListRestraint::get_interacting_particles() const
{
  ParticlesList ret(get_number_of_particle_pairs());
  for (unsigned int i=0; i< get_number_of_particle_pairs(); ++i) {
    ret[i]= Particles(2);
    ret[i][0]= get_particle_pair(i).first;
    ret[i][1]= get_particle_pair(i).second;
  }
  return ret;
}

void PairListRestraint::show(std::ostream& out) const
{
  out << "Pair list restraint with score function ";
  ss_->show(out);
  out << std::endl;
}

IMP_LIST_IMPL(PairListRestraint, ParticlePair, particle_pair, ParticlePair,,,)

IMPCORE_END_NAMESPACE

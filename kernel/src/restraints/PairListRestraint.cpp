/**
 *  \file PairListRestraint.cpp
 *  \brief Apply a score function to a list of pairs of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/PairListRestraint.h"
#include "IMP/PairScore.h"
#include "IMP/log.h"

#include <cmath>

IMP_BEGIN_NAMESPACE

PairListRestraint::PairListRestraint(PairScore *s,
                                     const ParticlePairs &ps) : ss_(s)
{
  add_particle_pairs(ps);
}

PairListRestraint::~PairListRestraint()
{
}

Float PairListRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(ss_.get());
  IMP_assert(get_number_of_particles()%2 == 0, "There should be an even number"
             << " of particles");
  Float score=0;

  for (unsigned int i=0; i< get_number_of_particles(); i+=2) {
    score += ss_->evaluate(get_particle(i), get_particle(i+1), accum);
  }

  return score;
}


void PairListRestraint::add_particle_pair(ParticlePair p)
{
  Restraint::add_particle(p.first);
  Restraint::add_particle(p.second);
}

void PairListRestraint::clear_particle_pairs()
{
  Restraint::clear_particles();
}

void PairListRestraint::add_particle_pairs(const ParticlePairs &ps)
{
  for (unsigned int i=0; i< ps.size(); ++i) {
    add_particle_pair(ps[i]);
  }
}

ParticlesList PairListRestraint::get_interacting_particles() const
{
  ParticlesList ret(get_number_of_particles()/2);
  for (unsigned int i=0; 2*i< get_number_of_particles(); ++i) {
    ret[i]= Particles(2);
    ret[i][0]= get_particle(i*2);
    ret[i][1]= get_particle(i*2+1);
  }
  return ret;
}

void PairListRestraint::show(std::ostream& out) const
{
  out << "Pair list restraint with score function ";
  ss_->show(out);
  out << std::endl;
}

IMP_END_NAMESPACE

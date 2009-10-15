/**
 *  \file RefinedPairsPairScore.cpp
 *  \brief Refine particles at most once with a Refiner.
 *
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/RefinedPairsPairScore.h>

#include <IMP/core/XYZ.h>
#include <IMP/internal/container_helpers.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

RefinedPairsPairScore::RefinedPairsPairScore(Refiner *r,
                                         PairScore *f): r_(r), f_(f) {}
namespace {
  ParticlesTemp get_set(Particle *a, Refiner *r) {
    ParticlesTemp ret;
    if (r->get_can_refine(a)) {
      ret= r->get_refined(a);
    } else {
      ret.push_back(a);
    }
    return ret;
  }
}

Float RefinedPairsPairScore::evaluate(Particle *a, Particle *b,
                                    DerivativeAccumulator *da) const
{
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};
  Float ret=0;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ret+=f_->evaluate(ps[0][i], ps[1][j], da);
    }
  }
  return ret;
}

ParticlesList
RefinedPairsPairScore::get_interacting_particles(Particle *a,
                                                 Particle *b) const {
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};
  ParticlesList ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ret.push_back(IMP::internal::get_union(IMP::internal
                            ::get_interacting_particles(ParticlePair(ps[0][i],
                                                                     ps[1][j]),
                                                          f_.get())));
    }
  }
  return ret;
}

ParticlesTemp RefinedPairsPairScore::get_read_particles(Particle *a,
                                                        Particle *b) const {
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ParticlesTemp cps
        = IMP::internal::get_read_particles(ParticlePair(ps[0][i],
                                                         ps[1][j]),
                                            f_.get());
      ret.insert(ret.end(), cps.begin(), cps.end());
    }
  }
  ret.push_back(a);
  ret.push_back(b);
  return ret;
}


ParticlesTemp RefinedPairsPairScore::get_write_particles(Particle *a,
                                                        Particle *b) const {
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ParticlesTemp cps
        = IMP::internal::get_write_particles(ParticlePair(ps[0][i],
                                                         ps[1][j]),
                                            f_.get());
      ret.insert(ret.end(), cps.begin(), cps.end());
    }
  }
  ret.push_back(a);
  ret.push_back(b);
  return ret;
}


void RefinedPairsPairScore::show(std::ostream &out) const
{
  out << "RefinedPairsPairScore using ";
  f_->show(out);
  r_->show(out);
}

IMPCORE_END_NAMESPACE

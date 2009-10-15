/**
 *  \file RefineOncePairScore.cpp
 *  \brief Refine particles at most once with a Refiner.
 *
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/misc/RefineOncePairScore.h>

#include <IMP/core/XYZ.h>
#include <IMP/deprecation.h>
#include <IMP/internal/container_helpers.h>
#include <cmath>

IMPMISC_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

RefineOncePairScore::RefineOncePairScore(Refiner *r,
                                         PairScore *f): r_(r), f_(f) {
  IMP_DEPRECATED(RefineOncePairScore, RefinedPairsPairScore);
}

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

Float RefineOncePairScore::evaluate(Particle *a, Particle *b,
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
RefineOncePairScore::get_interacting_particles(Particle *a,
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

ParticlesTemp RefineOncePairScore::get_read_particles(Particle *a,
                                                      Particle *b) const {
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ParticlesTemp cps= IMP::internal
        ::get_read_particles(ParticlePair(ps[0][i],
                                          ps[1][j]), f_.get());
      ret.insert(ret.end(), cps.begin(), cps.end());
    }
  }
  ret.push_back(a);
  ret.push_back(b);
  return ret;
}

ParticlesTemp RefineOncePairScore::get_write_particles(Particle *a,
                                                       Particle *b) const {
  // lazy
  return get_read_particles(a,b);
}


void RefineOncePairScore::show(std::ostream &out) const
{
  out << "RefineOncePairScore using ";
  f_->show(out);
  r_->show(out);
}

#endif // IMP_NO_DEPRECATED

IMPMISC_END_NAMESPACE

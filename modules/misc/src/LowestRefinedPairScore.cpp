/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Lowest particles at most refined with a ParticleLowestr.
 *
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/misc/LowestRefinedPairScore.h>

#include <IMP/core/XYZ.h>
#include <IMP/internal/container_helpers.h>
#include <cmath>

IMPMISC_BEGIN_NAMESPACE

LowestRefinedPairScore::LowestRefinedPairScore(Refiner *r,
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

  std::pair<double, ParticlePair> get_lowest(Particles ps[2],
                                             PairScore* f) {
    double ret=std::numeric_limits<Float>::max();
    ParticlePair lowest;
    for (unsigned int i=0; i< ps[0].size(); ++i) {
      for (unsigned int j=0; j< ps[1].size(); ++j) {
        Float v= f->evaluate(ps[0][i], ps[1][j], NULL);
        if (v < ret) {
          ret=v;
          lowest= ParticlePair(ps[0][i], ps[1][j]);
        }
      }
    }
    return std::make_pair(ret, lowest);
  }
}


Float LowestRefinedPairScore::evaluate(Particle *a, Particle *b,
                                    DerivativeAccumulator *da) const
{
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};

  std::pair<double, ParticlePair> r= get_lowest(ps, f_);

  if (da) {
    f_->evaluate(r.second.first, r.second.second, da);
  }

  return r.first;
}


ParticlesList
LowestRefinedPairScore::get_interacting_particles(Particle *a,
                                                  Particle *b) const {
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};

  std::pair<double, ParticlePair> r= get_lowest(ps, f_);
  return f_->get_interacting_particles(r.second.first, r.second.second);
}

ParticlesTemp LowestRefinedPairScore::get_used_particles(Particle *a,
                                                         Particle *b) const {
  Particles ps[2]={get_set(a, r_), get_set(b, r_)};
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ParticlesTemp cpt= f_->get_used_particles(ps[0][i], ps[1][j]);
      ret.insert(ret.end(), cpt.begin(), cpt.end());
    }
  }
  ret.push_back(a);
  ret.push_back(b);
  return ret;
}

void LowestRefinedPairScore::show(std::ostream &out) const
{
  out << "LowestRefinedPairScore using ";
  f_->show(out);
  r_->show(out);
}

IMPMISC_END_NAMESPACE

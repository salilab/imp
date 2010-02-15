/**
 *  \file RefinedPairsPairScore.cpp
 *  \brief Refine particles at most once with a Refiner.
 *
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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

bool RefinedPairsPairScore::get_is_changed(const ParticlePair &p) const {
  for (unsigned int i=0; i< 2; ++i) {
    ParticlesTemp ps=get_set(p[0], r_);
    for (unsigned int j=0; j< ps.size(); ++j) {
      if (ps[i]->get_is_changed()) return true;
    }
  }
  return false;
}

Float RefinedPairsPairScore::evaluate(const ParticlePair &p,
                                    DerivativeAccumulator *da) const
{
  ParticlesTemp ps[2]={get_set(p[0], r_), get_set(p[1], r_)};
  Float ret=0;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ret+=f_->evaluate(ParticlePair(ps[0][i], ps[1][j]), da);
    }
  }
  return ret;
}

ParticlesList
RefinedPairsPairScore::get_interacting_particles(const ParticlePair &p) const {
  ParticlesTemp ps[2]={get_set(p[0], r_), get_set(p[1], r_)};
  ParticlesList ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ParticlePair pp(ps[0][i],ps[1][j]);
      ret.push_back(IMP::internal::get_union(
                       f_->get_interacting_particles(pp)));
    }
  }
  return ret;
}

ParticlesTemp
RefinedPairsPairScore::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp ps[2]={get_set(p[0], r_), get_set(p[1], r_)};
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ParticlesTemp cps
        = f_->get_input_particles(ParticlePair(ps[0][i],
                                               ps[1][j]));
      ret.insert(ret.end(), cps.begin(), cps.end());
    }
  }
  ret.push_back(p[0]);
  ret.push_back(p[1]);
  ParticlesTemp i0= r_->get_input_particles(p[0]);
  ParticlesTemp i1= r_->get_input_particles(p[1]);
  ret.insert(ret.end(), i0.begin(), i0.end());
  ret.insert(ret.end(), i1.begin(), i1.end());
  return ret;
}


ContainersTemp
RefinedPairsPairScore::get_input_containers(const ParticlePair &p) const {
  ContainersTemp ret= r_->get_input_containers(p[0]);
  ContainersTemp i1= r_->get_input_containers(p[1]);
  ret.insert(ret.end(), i1.begin(), i1.end());
  return ret;
}




void RefinedPairsPairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
  out << "refiner " << *r_ << std::endl;
}

IMPCORE_END_NAMESPACE

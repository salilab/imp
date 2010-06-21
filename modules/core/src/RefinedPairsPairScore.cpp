/**
 *  \file RefinedPairsPairScore.cpp
 *  \brief Refine particles at most once with a Refiner.
 *
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
      IMP_USAGE_CHECK(ret.size() > 0,
                      "The refiner did not return any particles for "
                      << a->get_name() << ". The refiner is "
                      << *r);
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
  double ret=0;
  for (unsigned int i=0; i< ps[0].size(); ++i) {
    for (unsigned int j=0; j< ps[1].size(); ++j) {
      ret+=f_->evaluate(ParticlePair(ps[0][i], ps[1][j]), da);
    }
  }
  return ret;
}

ParticlesTemp
RefinedPairsPairScore::get_input_particles(Particle *p) const {
  ParticlesTemp ps=get_set(p, r_);
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    ParticlesTemp cps
      = f_->get_input_particles(ps[i]);
    ret.insert(ret.end(), cps.begin(), cps.end());
  }
  ret.push_back(p);
  ParticlesTemp i0= r_->get_input_particles(p);
  ret.insert(ret.end(), i0.begin(), i0.end());
  return ret;
}


ContainersTemp
RefinedPairsPairScore::get_input_containers(Particle *p) const {
  ContainersTemp ret= r_->get_input_containers(p);
  return ret;
}




void RefinedPairsPairScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
  out << "refiner " << *r_ << std::endl;
}

IMPCORE_END_NAMESPACE

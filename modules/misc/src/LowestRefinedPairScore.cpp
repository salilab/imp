/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Lowest particles at most refined with a ParticleLowestr.
 *
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
        Float v= f->evaluate(ParticlePair(ps[0][i], ps[1][j]), NULL);
        if (v < ret) {
          ret=v;
          lowest= ParticlePair(ps[0][i], ps[1][j]);
        }
      }
    }
    return std::make_pair(ret, lowest);
  }
}


bool LowestRefinedPairScore::get_is_changed(const ParticlePair &p) const {
  for (unsigned int i=0; i< 2; ++i) {
    ParticlesTemp ps=get_set(p[0], r_);
    for (unsigned int j=0; j< ps.size(); ++j) {
      if (ps[i]->get_is_changed()) return true;
    }
  }
  return false;
}

Float LowestRefinedPairScore::evaluate(const ParticlePair &p,
                                    DerivativeAccumulator *da) const
{
  Particles ps[2]={get_set(p[0], r_), get_set(p[1], r_)};

  std::pair<double, ParticlePair> r= get_lowest(ps, f_);

  if (da) {
    f_->evaluate(r.second, da);
  }

  return r.first;
}



ParticlesTemp LowestRefinedPairScore
::get_input_particles(Particle *p) const {
  ParticlesTemp ps=get_set(p, r_);
  ParticlesTemp ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    ParticlesTemp cpt= f_->get_input_particles(ps[i]);
    ret.insert(ret.end(), cpt.begin(), cpt.end());
  }
  ret.push_back(p);
  ParticlesTemp ia= r_->get_input_particles(p);
  ret.insert(ret.end(), ia.begin(), ia.end());
  return ret;
}

ContainersTemp LowestRefinedPairScore
::get_input_containers(Particle *p) const {
  ContainersTemp ret= r_->get_input_containers(p);
  return ret;
}


void LowestRefinedPairScore::do_show(std::ostream &out) const
{
  f_->show(out);
  r_->show(out);
}

IMPMISC_END_NAMESPACE

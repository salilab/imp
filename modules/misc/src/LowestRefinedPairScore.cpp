/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Lowest particles at most refined with a ParticleLowestr.
 *
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

  std::pair<double, ParticlePair> get_lowest(ParticlesTemp ps[2],
                                             PairScore* f) {
    double ret=std::numeric_limits<Float>::max();
    ParticlePair lowest;
    for (unsigned int i=0; i< ps[0].size(); ++i) {
      for (unsigned int j=0; j< ps[1].size(); ++j) {
        Float v= f->evaluate_index(ps[0][0]->get_model(),
                                   ParticleIndexPair(ps[0][i]->get_index(),
                                                     ps[1][j]->get_index()),
                                   nullptr);
        if (v < ret) {
          ret=v;
          lowest= ParticlePair(ps[0][i], ps[1][j]);
        }
      }
    }
    return std::make_pair(ret, lowest);
  }
}

Float LowestRefinedPairScore::evaluate_index(Model *m,
                                             const ParticleIndexPair &pi,
                                             DerivativeAccumulator *da) const
{
  ParticlesTemp ps[2]={get_set(m->get_particle(pi[0]), r_),
                       get_set(m->get_particle(pi[1]), r_)};

  std::pair<double, ParticlePair> r= get_lowest(ps, f_);

  if (da) {
    f_->evaluate_index(m, ParticleIndexPair(r.second[0]->get_index(),
                                            r.second[1]->get_index()), da);
  }

  return r.first;
}



ModelObjectsTemp LowestRefinedPairScore
::do_get_inputs(Model *m,
                const ParticleIndexes &pis) const {
  return r_->get_inputs(m, pis) + f_->get_inputs(m, pis);
}


void LowestRefinedPairScore::do_show(std::ostream &out) const
{
  f_->show(out);
  r_->show(out);
}

IMPMISC_END_NAMESPACE

/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Score on the lowest scoring pair of the refined pairs.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/misc/LowestRefinedPairScore.h>

#include <IMP/core/XYZ.h>
#include <IMP/kernel/internal/container_helpers.h>
#include <cmath>

IMPMISC_BEGIN_NAMESPACE

LowestRefinedPairScore::LowestRefinedPairScore(Refiner *r, PairScore *f)
    : r_(r), f_(f) {}

namespace {
ParticlesTemp get_set(kernel::Particle *a, Refiner *r) {
  kernel::ParticlesTemp ret;
  if (r->get_can_refine(a)) {
    ret = r->get_refined(a);
  } else {
    ret.push_back(a);
  }
  return ret;
}

std::pair<double, kernel::ParticlePair> get_lowest(kernel::ParticlesTemp ps[2],
                                                   PairScore *f) {
  double ret = std::numeric_limits<Float>::max();
  kernel::ParticlePair lowest;
  for (unsigned int i = 0; i < ps[0].size(); ++i) {
    for (unsigned int j = 0; j < ps[1].size(); ++j) {
      Float v =
          f->evaluate_index(ps[0][0]->get_model(),
                            kernel::ParticleIndexPair(ps[0][i]->get_index(),
                                                      ps[1][j]->get_index()),
                            nullptr);
      if (v < ret) {
        ret = v;
        lowest = kernel::ParticlePair(ps[0][i], ps[1][j]);
      }
    }
  }
  return std::make_pair(ret, lowest);
}
}

Float LowestRefinedPairScore::evaluate_index(
    kernel::Model *m, const kernel::ParticleIndexPair &pi,
    DerivativeAccumulator *da) const {
  kernel::ParticlesTemp ps[2] = {get_set(m->get_particle(pi[0]), r_),
                                 get_set(m->get_particle(pi[1]), r_)};

  std::pair<double, kernel::ParticlePair> r = get_lowest(ps, f_);

  if (da) {
    f_->evaluate_index(m, kernel::ParticleIndexPair(r.second[0]->get_index(),
                                                    r.second[1]->get_index()),
                       da);
  }

  return r.first;
}

ModelObjectsTemp LowestRefinedPairScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret = r_->get_inputs(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (r_->get_can_refine(m->get_particle(pis[i]))) {
      kernel::ParticleIndexes cur = r_->get_refined_indexes(m, pis[i]);
      ret += f_->get_inputs(m, cur);
    }
  }
  return ret;
}

IMPMISC_END_NAMESPACE

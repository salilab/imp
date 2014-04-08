/**
 *  \file RefinedPairsPairScore.cpp
 *  \brief Refine particles at most once with a Refiner.
 *
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/core/RefinedPairsPairScore.h>

#include <IMP/core/XYZ.h>
#include <IMP/kernel/internal/container_helpers.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

RefinedPairsPairScore::RefinedPairsPairScore(Refiner *r, PairScore *f)
    : r_(r), f_(f) {}
namespace {
ParticlesTemp get_set(kernel::Particle *a, Refiner *r) {
  kernel::ParticlesTemp ret;
  if (r->get_can_refine(a)) {
    ret = r->get_refined(a);
    IMP_USAGE_CHECK(ret.size() > 0,
                    "The refiner did not return any particles for "
                        << a->get_name() << ". The refiner is " << *r);
  } else {
    ret.push_back(a);
  }
  return ret;
}
}

Float RefinedPairsPairScore::evaluate_index(kernel::Model *m,
                                            const kernel::ParticleIndexPair &p,
                                            DerivativeAccumulator *da) const {
  kernel::ParticlesTemp ps[2] = {get_set(m->get_particle(p[0]), r_),
                                 get_set(m->get_particle(p[1]), r_)};
  double ret = 0;
  for (unsigned int i = 0; i < ps[0].size(); ++i) {
    for (unsigned int j = 0; j < ps[1].size(); ++j) {
      ret +=
          f_->evaluate_index(ps[0][0]->get_model(),
                             kernel::ParticleIndexPair(ps[0][i]->get_index(),
                                                       ps[1][j]->get_index()),
                             da);
    }
  }
  return ret;
}

ModelObjectsTemp RefinedPairsPairScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ParticleIndexes ps;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ps += get_indexes(get_set(m->get_particle(pis[i]), r_));
  }
  kernel::ModelObjectsTemp ret;
  ret += f_->get_inputs(m, ps);
  ret += r_->get_inputs(m, ps);
  return ret;
}

IMPCORE_END_NAMESPACE

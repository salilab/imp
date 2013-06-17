/**
 *  \file BondSingletonScore.cpp
 *  \brief A score based a bond decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include "IMP/atom/BondSingletonScore.h"
#include "IMP/core/internal/evaluate_distance_pair_score.h"
#include "IMP/atom/bond_decorators.h"
#include "IMP/core/XYZ.h"

#include <boost/lambda/lambda.hpp>

IMPATOM_BEGIN_NAMESPACE

BondSingletonScore::BondSingletonScore(UnaryFunction *f) : f_(f) {}

double BondSingletonScore::evaluate_index(Model *m, ParticleIndex pi,
                                          DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(Bond::particle_is_instance(m, pi),
                  "Particle is not a bond particle");
  Bond bd(m, pi);
  Float l = bd.get_length();
  Float s = bd.get_stiffness();
  if (l < 0) {
    IMP_WARN("Bond does not have a length: " << bd << std::endl);
    return 0;
  }
  if (s < 0) s = 1;
  Particle *pa = nullptr, *pb = nullptr;
  try {
    pa = bd.get_bonded(0).get_particle();
    pb = bd.get_bonded(1).get_particle();
  }
  catch (const base::IndexException & e) {
    IMP_WARN("Problem processing bond: " << bd << std::endl);
    IMP_WARN(e.what() << std::endl);
    return 0;
  }
  return IMP::core::internal::evaluate_distance_pair_score(
      IMP::core::XYZ(pa), IMP::core::XYZ(pb), da, f_.get(),
      s * (boost::lambda::_1 - l), s);
}

ModelObjectsTemp BondSingletonScore::do_get_inputs(Model *m,
                                                   const ParticleIndexes &pi)
    const {
  ModelObjectsTemp ret(3*pi.size());
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Bond ad(m, pi[i]);
    ret[3 * i + 0] = ad.get_bonded(0);
    ret[3 * i + 1] = ad.get_bonded(1);
    ret[3 * i + 3] = m->get_particle(pi[i]);
  }
  return ret;
}

IMPATOM_END_NAMESPACE

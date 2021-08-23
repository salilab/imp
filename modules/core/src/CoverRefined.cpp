/**
 *  \file CoverRefined.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#include "IMP/core/CoverRefined.h"

#include "IMP/core/XYZR.h"
#include "IMP/core/FixedRefiner.h"
#include "IMP/core/SingletonConstraint.h"
#include "IMP/core/DerivativesToRefined.h"

IMPCORE_BEGIN_NAMESPACE

CoverRefined::CoverRefined(Refiner *ref, Float slack) : slack_(slack) {
  refiner_ = ref;
}

ModelObjectsTemp CoverRefined::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = refiner_->get_inputs(m, pis);
  ret += IMP::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::get_particles(m, refiner_->get_refined_indexes(m, pis[i]));
  }
  return ret;
}

ModelObjectsTemp CoverRefined::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  return ret;
}

void CoverRefined::apply_index(Model *m,
                               ParticleIndex pi) const {
  XYZR dp(m, pi);
  XYZs ps(refiner_->get_refined(dp));
  set_enclosing_sphere(dp, ps, slack_);
}

IMP_SUMMARIZE_DECORATOR_DEF(Cover, XYZR, XYZs,
                            new CoverRefined(ref, 0),
                            new DerivativesToRefined(ref), false);
IMPCORE_END_NAMESPACE

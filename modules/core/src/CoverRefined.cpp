/**
 *  \file CoverRefined.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

ModelObjectsTemp CoverRefined::do_get_inputs(kernel::Model *m,
                                             const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret = refiner_->get_inputs(m, pis);
  ret += IMP::kernel::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::kernel::get_particles(m, refiner_->get_refined_indexes(m, pis[i]));
  }
  return ret;
}

ModelObjectsTemp CoverRefined::do_get_outputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret = IMP::kernel::get_particles(m, pis);
  return ret;
}

void CoverRefined::apply_index(kernel::Model *m, kernel::ParticleIndex pi) const {
  XYZR dp(m, pi);
  XYZs ps(refiner_->get_refined(dp));
  set_enclosing_sphere(dp, ps, slack_);
}

IMP_SUMMARY_DECORATOR_DEF(Cover, XYZR, XYZs,
                          SingletonModifier *mod = new CoverRefined(ref, 0));
IMPCORE_END_NAMESPACE

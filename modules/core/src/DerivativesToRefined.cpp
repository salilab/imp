/**
 *  \file DerivativesToRefined.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include "IMP/core/DerivativesToRefined.h"
#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

DerivativesToRefined::DerivativesToRefined(Refiner *r, FloatKeys ks)
    : refiner_(r), ks_(ks) {}

void DerivativesToRefined::apply_index(kernel::Model *m,
                                       kernel::ParticleIndex pi) const {
  kernel::Particle *p = m->get_particle(pi);
  DerivativeAccumulator da;
  kernel::ParticlesTemp ps = refiner_->get_refined(p);

  for (unsigned int i = 0; i < ps.size(); ++i) {
    for (unsigned int j = 0; j < ks_.size(); ++j) {
      Float f = p->get_derivative(ks_[j]);
      ps[i]->add_to_derivative(ks_[j], f, da);
    }
  }
}

ModelObjectsTemp DerivativesToRefined::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret = refiner_->get_inputs(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::kernel::get_particles(m, refiner_->get_refined_indexes(m, pis[i]));
  }
  ret += IMP::kernel::get_particles(m, pis);
  return ret;
}

ModelObjectsTemp DerivativesToRefined::do_get_outputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::kernel::get_particles(m, refiner_->get_refined_indexes(m, pis[i]));
  }
  return ret;
}

IMPCORE_END_NAMESPACE

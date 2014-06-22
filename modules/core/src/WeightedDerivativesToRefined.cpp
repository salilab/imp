/**
 *  \file WeightedDerivativesToRefined.cpp
 *  \brief Copies derivatives from coarse grained particle
 *         to its refined set of particles using weights
 *         indicated by a weight key.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include "IMP/core/WeightedDerivativesToRefined.h"
#include <IMP/core/XYZ.h>
#include <IMP/base_types.h>
#include <IMP/DerivativeAccumulator.h>

IMPCORE_BEGIN_NAMESPACE

WeightedDerivativesToRefined::WeightedDerivativesToRefined
( Refiner *r, FloatKey w, FloatKeys keys)
  : refiner_(r), w_(w), keys_(keys) {}

void WeightedDerivativesToRefined::apply_index(kernel::Model *m,
                                       kernel::ParticleIndex pi) const {
  // retrieving pis by ref if possible is cumbersome but is required for speed
  kernel::ParticleIndexes pis_if_not_byref;
  kernel::ParticleIndexes const* pPis;
  if(refiner_->get_is_by_ref_supported()){
    kernel::ParticleIndexes const& pis =
      refiner_->get_refined_indexes_by_ref(m, pi);
    pPis = &pis;
  } else{
    pis_if_not_byref = refiner_->get_refined_indexes(m, pi);
    pPis = &pis_if_not_byref;
  }
  kernel::ParticleIndexes const& pis = *pPis;
  //  Prepare derivative accumulator to normalize by total weight
  Float total_weight;
  if(w_ != FloatKey()){
    total_weight = m->get_attribute(w_, pi);
  } else {
    total_weight = pis.size();
  }
  kernel::DerivativeAccumulator da( 1.0 / total_weight);
  // read K values for each key in keys_
  Floats Ks(keys_.size());
  for (unsigned int j = 0; j < Ks.size(); ++j){
    Ks[j] = m->get_derivative(keys_[j], pi);
  }
  // store K reweighted per each particle, normalized with da
  for (unsigned int i = 0; i < pis.size(); ++i) {
    Float w = m->get_attribute(w_, pis[i]);
    for (unsigned int j = 0; j < keys_.size(); ++j) {
      m->add_to_derivative(keys_[j], pis[i], w * Ks[j], da);
    }
  }
}

ModelObjectsTemp WeightedDerivativesToRefined::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret = refiner_->get_inputs(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::kernel::get_particles(m, refiner_->get_refined_indexes(m, pis[i]));
  }
  ret += IMP::kernel::get_particles(m, pis);
  return ret;
}

ModelObjectsTemp WeightedDerivativesToRefined::do_get_outputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret +=
        IMP::kernel::get_particles(m, refiner_->get_refined_indexes(m, pis[i]));
  }
  return ret;
}

IMPCORE_END_NAMESPACE

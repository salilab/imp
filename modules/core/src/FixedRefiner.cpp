/**
 *  \file FixedRefiner.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/FixedRefiner.h"
#include <IMP/base/log.h>

IMPCORE_BEGIN_NAMESPACE

FixedRefiner::FixedRefiner(const kernel::ParticlesTemp &ps)
  : Refiner("FixedRefiner%d", true) {
  IMP_USAGE_CHECK(ps.size()>0, "cannot refine with empty particle list");
  IMP_LOG_VERBOSE("Created fixed particle refiner with "
                  << ps.size() << " particles" << std::endl);
  m_ = ps[0]->get_model();
  for(unsigned int i = 0; i < ps.size(); i++){
    IMP_USAGE_CHECK(m_ == ps[i]->get_model(),
                    "refiner assumes all particles are from the same model");
    pis_.push_back(ps[i]->get_index());
  }
}

FixedRefiner::FixedRefiner(Model* m, const kernel::ParticleIndexes &pis)
  : Refiner("FixedRefiner%d", true), m_(m), pis_(pis) {
  IMP_LOG_VERBOSE("Created fixed particle refiner with "
                  << pis_.size() << " particles" << std::endl);
}

const kernel::ParticlesTemp
FixedRefiner::get_refined(kernel::Particle *) const
{
  ParticlesTemp ps;
  for(unsigned int i=0; i < pis_.size(); i++){
    ps.push_back(m_->get_particle(pis_[i]));
  }
  return ps;
}


ModelObjectsTemp FixedRefiner::do_get_inputs(
    kernel::Model *, const kernel::ParticleIndexes &) const {
  return kernel::ModelObjectsTemp();
}

IMPCORE_END_NAMESPACE

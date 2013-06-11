/**
 *  \file FixedRefiner.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/FixedRefiner.h"
#include <IMP/base/log.h>

IMPCORE_BEGIN_NAMESPACE

FixedRefiner::FixedRefiner(const ParticlesTemp &ps)
    : Refiner("FixedRefiner%d"), ps_(ps) {
  IMP_LOG_VERBOSE("Created fixed particle refiner with "
                  << ps.size() << " particles" << std::endl);
}

bool FixedRefiner::get_can_refine(Particle *) const { return true; }

const ParticlesTemp FixedRefiner::get_refined(Particle *) const { return ps_; }

ModelObjectsTemp FixedRefiner::do_get_inputs(Model *,
                                             const ParticleIndexes &) const {
  return ModelObjectsTemp();
}

IMPCORE_END_NAMESPACE

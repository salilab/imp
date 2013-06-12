/**
 *  \file ClosePairsFinder.cpp
 *  \brief Algorithm base class to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/ClosePairsFinder.h"
#include "IMP/core/internal/MovedSingletonContainer.h"
#include <limits>

IMPCORE_BEGIN_NAMESPACE

ClosePairsFinder::ClosePairsFinder(std::string name)
    : Object(name), distance_(std::numeric_limits<double>::quiet_NaN()) {
  set_was_used(true);
}

ClosePairsFinder::~ClosePairsFinder() {}

internal::MovedSingletonContainer*
ClosePairsFinder::get_moved_singleton_container(SingletonContainer* in,
                                                double threshold) const {
  return new internal::XYZRMovedSingletonContainer(in, threshold);
}

ParticlePairsTemp
ClosePairsFinder::get_close_pairs(const ParticlesTemp &pc) const {
  return IMP::internal::get_particle(IMP::internal::get_model(pc),
                            get_close_pairs(IMP::internal::get_model(pc),
                                            IMP::internal::get_index(pc)));
}

ParticlePairsTemp
ClosePairsFinder::get_close_pairs(const ParticlesTemp &pca,
                                  const ParticlesTemp &pcb) const {
  return IMP::internal::get_particle(IMP::internal::get_model(pca),
                            get_close_pairs(IMP::internal::get_model(pca),
                                            IMP::internal::get_index(pca),
                                            IMP::internal::get_index(pcb)));
}

IMP_LIST_IMPL(ClosePairsFinder, PairFilter, pair_filter, PairPredicate*,
              PairPredicates);

IMPCORE_END_NAMESPACE

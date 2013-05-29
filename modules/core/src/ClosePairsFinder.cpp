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

IMP_LIST_IMPL(ClosePairsFinder, PairFilter, pair_filter, PairPredicate*,
              PairPredicates);
IMP_INPUTS_DEF(ClosePairsFinder);

IMPCORE_END_NAMESPACE

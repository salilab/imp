/**
 *  \file ClosePairsFinder.cpp
 *  \brief Algorithm base class to find close pairs.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/ClosePairsFinder.h"
#include "IMP/core/ListSingletonContainer.h"
#include "IMP/core/internal/MovedSingletonContainer.h"
#include <limits>

IMPCORE_BEGIN_NAMESPACE

ClosePairsFinder::ClosePairsFinder(std::string name): Object(name),
  distance_(std::numeric_limits<double>::quiet_NaN()){}

ClosePairsFinder::~ClosePairsFinder(){}

internal::MovedSingletonContainer*
ClosePairsFinder::get_moved_singleton_container(SingletonContainer *in,
                                                 Model *m,
                                                 double threshold) const {
  return new internal::MovedSingletonContainerImpl<algebra::Sphere3D,
    internal::SaveXYZRValues,
    internal::SaveMovedValues<internal::SaveXYZRValues>,
    internal::ListXYZRMovedParticles>(m, in, threshold);
}


IMPCORE_END_NAMESPACE

/**
 *  \file ClosePairsFinder.cpp
 *  \brief Algorithm base class to find close pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/ClosePairsFinder.h"
#include "IMP/core/ListSingletonContainer.h"
#include <limits>

IMPCORE_BEGIN_NAMESPACE

ClosePairsFinder::ClosePairsFinder():
  distance_(std::numeric_limits<double>::quiet_NaN()){}

ClosePairsFinder::~ClosePairsFinder(){}

IMPCORE_END_NAMESPACE

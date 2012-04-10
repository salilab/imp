/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/BondedPairFilter.h"

IMPATOM_BEGIN_NAMESPACE

BondedPairFilter
::BondedPairFilter(): PairPredicate("BondedPairFilter%1%"){
}

IMPATOM_END_NAMESPACE

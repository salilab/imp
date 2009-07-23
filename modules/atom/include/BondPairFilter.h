/**
 *  \file atom/BondPairFilter.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_PAIR_FILTER_H
#define IMPATOM_BOND_PAIR_FILTER_H

#include "config.h"
#include "bond_decorators.h"
#include "internal/version_info.h"

#include <IMP/PairContainer.h>

IMPATOM_BEGIN_NAMESPACE

//! A "container" that pretends to contain all bonds.
/** This is to be used with a core::ClosePairsScoreState to exclude all
    bonded pairs.
    \ingroup bond
    \see Bonded
 */
class IMPATOMEXPORT BondPairFilter :
  public PairContainer
{
public:
  //! no arguments
  BondPairFilter();

  IMP_PAIR_CONTAINER(BondPairFilter, internal::version_info)
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_PAIR_FILTER_H */

/**
 *  \file BondPairContainer.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_PAIR_CONTAINER_H
#define IMPCORE_BOND_PAIR_CONTAINER_H

#include "config.h"
#include "bond_decorators.h"
#include "internal/version_info.h"

#include <IMP/PairContainer.h>

IMPCORE_BEGIN_NAMESPACE

//! A container that pretends to contain all bonds.
/** This is to be used with a ClosePairsScoreState to exclude all bonded pairs.
    \ingroup bond
 */
class IMPCOREEXPORT BondPairContainer :
  public PairContainer
{
public:
  //! no arguments
  BondPairContainer();

  virtual ~BondPairContainer();

  IMP_PAIR_CONTAINER(internal::version_info)
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_PAIR_CONTAINER_H */

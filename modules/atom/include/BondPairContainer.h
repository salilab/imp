/**
 *  \file BondPairContainer.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_PAIR_CONTAINER_H
#define IMPATOM_BOND_PAIR_CONTAINER_H

#include "config.h"
#include "bond_decorators.h"
#include "internal/version_info.h"

#include <IMP/PairContainer.h>

IMPATOM_BEGIN_NAMESPACE

//! A container that pretends to contain all bonds.
/** This is to be used with a ClosePairsScoreState to exclude all bonded pairs.
    \ingroup bond
    \see BondedDecorator
 */
class IMPATOMEXPORT BondPairContainer :
  public PairContainer
{
public:
  //! no arguments
  BondPairContainer();

  virtual ~BondPairContainer();

  IMP_PAIR_CONTAINER(internal::version_info)
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_PAIR_CONTAINER_H */

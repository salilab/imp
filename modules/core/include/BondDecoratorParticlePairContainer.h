/**
 *  \file BondDecoratorParticlePairContainer.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_DECORATOR_PARTICLE_PAIR_CONTAINER_H
#define IMPCORE_BOND_DECORATOR_PARTICLE_PAIR_CONTAINER_H

#include "core_exports.h"
#include "ParticlePairContainer.h"
#include "bond_decorators.h"
#include "internal/core_version_info.h"

IMPCORE_BEGIN_NAMESPACE

//! A container that pretends to contain all bonds.
/** This is to be used with a NonBondedList to exclude all bonded pairs.
 */
class IMPCOREEXPORT BondDecoratorParticlePairContainer :
  public ParticlePairContainer
{
public:
  BondDecoratorParticlePairContainer();

  virtual ~BondDecoratorParticlePairContainer();

  IMP_PARTICLE_PAIR_CONTAINER(internal::core_version_info)
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_DECORATOR_PARTICLE_PAIR_CONTAINER_H */

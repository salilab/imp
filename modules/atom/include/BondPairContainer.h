/**
 *  \file atom/BondPairContainer.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_PAIR_CONTAINER_H
#define IMPATOM_BOND_PAIR_CONTAINER_H

#include "config.h"
#include "bond_decorators.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>

IMPATOM_BEGIN_NAMESPACE

//! A container that returns pairs of the endpoints of the bonds.
/** Turn a container of Bond particles into a container of ParticlePair
    objects of the endpoints.
    \ingroup bond
    \see Bonded
 */
class IMPATOMEXPORT BondPairContainer :
  public PairContainer
{
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  BondPairContainer(SingletonContainer *sc, bool);
public:
  //! The container containing the bonds
  BondPairContainer(SingletonContainer *sc);

  static BondPairContainer *create_untracked_container(SingletonContainer *c) {
    BondPairContainer *lsc = new BondPairContainer(c, false);
    return lsc;
  }

  IMP_PAIR_CONTAINER(BondPairContainer, get_module_version_info())
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_PAIR_CONTAINER_H */

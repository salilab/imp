/**
 *  \file atom/BondPairContainer.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_BOND_PAIR_CONTAINER_H
#define IMPATOM_BOND_PAIR_CONTAINER_H

#include "atom_config.h"
#include "bond_decorators.h"

#include <IMP/PairContainer.h>
#include <IMP/PairModifier.h>
#include <IMP/PairScore.h>
#include <IMP/SingletonContainer.h>
#include <IMP/scoped.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/core/PairRestraint.h>

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
  IMP::OwnerPointer<SingletonContainer> sc_;
  IMP_CONTAINER_DEPENDENCIES(BondPairContainer, ret.push_back(back_->sc_););
  BondPairContainer(SingletonContainer *sc, bool);

#define IMP_BP_LOOP(body)                                               \
  IMP_FOREACH_SINGLETON_INDEX(sc_, {                                    \
    Bond bp(get_model(), _1);                                           \
    ParticleIndexPair item(bp.get_bonded(0).get_particle_index(),       \
                           bp.get_bonded(1).get_particle_index());      \
    body;                                                               \
    }                                                                   \
    );
 IMP_IMPLEMENT_PAIR_CONTAINER_OPERATIONS(BondPairContainer,
                                               IMP_BP_LOOP);

#undef IMP_BP_LOOP
public:
  //! The container containing the bonds
  BondPairContainer(SingletonContainer *sc);

  static BondPairContainer *create_untracked_container(SingletonContainer *c) {
    BondPairContainer *lsc = new BondPairContainer(c, false);
    return lsc;
  }

#ifndef IMP_DOXYGEN
  bool get_is_up_to_date() const {
    return sc_->get_is_up_to_date();
  }
#endif
  IMP_PAIR_CONTAINER(BondPairContainer);
};

IMP_OBJECTS(BondPairContainer,BondPairContainers);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_PAIR_CONTAINER_H */

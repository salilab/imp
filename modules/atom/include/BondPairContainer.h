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
#include <IMP/internal/container_helpers.h>

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
  IMP_CONTAINER_DEPENDENCIES(BondPairContainer, ret.push_back(back_->sc_););
  BondPairContainer(SingletonContainer *sc, bool);
  template <class F>
    void apply_to_contents(F f) const {
    unsigned int sz= BondPairContainer::get_number_of_particle_pairs();
    for (unsigned int i=0; i< sz; ++i) {
      ParticlePair p= BondPairContainer::get_particle_pair(i);
      f(p);
    }
  }

  template <class F>
    double accumulate_over_contents(F f) const {
    double ret=0;
    unsigned int sz= BondPairContainer::get_number_of_particle_pairs();
    for (unsigned int i=0; i< sz; ++i) {
      ParticlePair p= BondPairContainer::get_particle_pair(i);
      ret+=f(p);
    }
    return ret;
  }
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

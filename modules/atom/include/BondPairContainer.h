/**
 *  \file IMP/atom/BondPairContainer.h
 *  \brief A fake container for bonds
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_BOND_PAIR_CONTAINER_H
#define IMPATOM_BOND_PAIR_CONTAINER_H

#include <IMP/atom/atom_config.h>
#include "bond_decorators.h"
#include <IMP/generic.h>
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
class IMPATOMEXPORT BondPairContainer : public PairContainer {
  IMP::PointerMember<SingletonContainer> sc_;

  virtual std::size_t do_get_contents_hash() const IMP_OVERRIDE {
    return sc_->get_contents_hash();
  }

 public:
  template <class F>
  void apply_generic(F* f) const {
    IMP_FOREACH(ParticleIndex pi, sc_->get_contents()) {
      Bond bp(get_model(), pi);
      f->apply_index(get_model(), ParticleIndexPair(
                                      bp.get_bonded(0).get_particle_index(),
                                      bp.get_bonded(1).get_particle_index()));
    }
  }

  //! The container containing the bonds
  BondPairContainer(SingletonContainer* sc);

  virtual ParticleIndexPairs get_indexes() const IMP_OVERRIDE;
  virtual ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return ModelObjects(1, sc_);
  }
  ParticleIndexes get_all_possible_indexes() const;
  IMP_PAIR_CONTAINER_METHODS(BondPairContainer);
  IMP_OBJECT_METHODS(BondPairContainer);
};

IMP_OBJECTS(BondPairContainer, BondPairContainers);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BOND_PAIR_CONTAINER_H */

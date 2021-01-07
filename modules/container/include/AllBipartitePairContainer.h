/**
 *  \file IMP/container/AllBipartitePairContainer.h
 *  \brief Return all bipartite pairs between two containers
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_ALL_BIPARTITE_PAIR_CONTAINER_H
#define IMPCONTAINER_ALL_BIPARTITE_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container/PairContainerSet.h>
#include <IMP/pair_macros.h>
#include <IMP/singleton_macros.h>
#include <boost/functional/hash/hash.hpp>

IMPCONTAINER_BEGIN_NAMESPACE

//! Return all bipartite pairs (a,b) for all a in container A
//! and b in container B
/** \see AllPairContainer, ClosePairContainer,
    CloseBipartitePairContainer for variants on the functionality provided.
 */
class IMPCONTAINEREXPORT AllBipartitePairContainer : public PairContainer {
  IMP::PointerMember<SingletonContainer> a_, b_;

 protected:
  virtual std::size_t do_get_contents_hash() const IMP_OVERRIDE {
    std::size_t ret = a_->get_contents_hash();
    boost::hash_combine(ret, b_->get_contents_hash());
    return ret;
  }

 public:
  template <class F>
  void apply_generic(F* f) const {
    // Note: currently assumes putting all pairs in memory is
    // feasible+reasonable but this could be easily made in chunks if
    // needed.
    validate_readable();
    ParticleIndexPairs pips;
    IMP_FOREACH(ParticleIndex pa, a_->get_contents()) {
      IMP_FOREACH(ParticleIndex pb, b_->get_contents()) {
        pips.push_back(ParticleIndexPair(pa, pb));
      }
    }
    f->apply_indexes(get_model(), pips,
                     0, pips.size());
  }
  AllBipartitePairContainer(SingletonContainerAdaptor a,
                            SingletonContainerAdaptor b,
                            std::string name = "AllBipartitePairContainer%1%");
  virtual ParticleIndexPairs get_indexes() const IMP_OVERRIDE;
  virtual ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  IMP_PAIR_CONTAINER_METHODS(AllBipartitePairContainer);
  IMP_OBJECT_METHODS(AllBipartitePairContainer);
};

IMP_OBJECTS(AllBipartitePairContainer, AllBipartitePairContainers);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_ALL_BIPARTITE_PAIR_CONTAINER_H */

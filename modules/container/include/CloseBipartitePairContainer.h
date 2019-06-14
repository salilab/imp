/**
 *  \file IMP/container/CloseBipartitePairContainer.h
 *  \brief Return all pairs from a SingletonContainer
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLOSE_BIPARTITE_PAIR_CONTAINER_H
#define IMPCONTAINER_CLOSE_BIPARTITE_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/core/ClosePairsFinder.h>
#include "internal/CloseBipartitePairContainer.h"
#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/PairContainer.h>
#include <IMP/PairPredicate.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

/** \brief Return all spatially-proximals pairs of particles (a,b) from the
    two SingletonContainers A and B, where a is in A and b is in B.

    \see ClosePairContainer for a more detailed description. This
    container lists all close pairs of particles where one particle is
    taken from each of the input sets.

    \see also AllPairContainer, ClosePairContainer,
    AllBipartitePairContainer for variants on the functionality provided.
 */
class IMPCONTAINEREXPORT CloseBipartitePairContainer :
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public PairContainer
#else
    public IMP::container::internal::CloseBipartitePairContainer
#endif
    {
  typedef IMP::container::internal::CloseBipartitePairContainer P;

 public:
  //! Get the individual particles from the passed SingletonContainer
  CloseBipartitePairContainer(SingletonContainerAdaptor a,
                              SingletonContainerAdaptor b, double distance,
                              double slack = 1,
                              std::string name =
                                  "CloseBipartitePairContainer%1%");
#ifndef IMP_DOXYGEN
  //! Get the individual particles from the passed SingletonContainer
  CloseBipartitePairContainer(SingletonContainerAdaptor a,
                              SingletonContainerAdaptor b, double distance,
                              core::ClosePairsFinder *cpf, double slack = 1,
                              std::string name =
                                  "CloseBipartitePairContainer%1%");
#endif

#if defined(IMP_DOXYGEN) || defined(SWIG)

  /** @name Methods to control the set of filters

      PairPredicate objects can be added as filters to prevent
      the addition of pairs to the container output list. Pairs
      for which the predicates evaluate to a non-zero value are
      excluded from the list.
  */
  /**@{*/
  IMP_LIST_ACTION(public, PairPredicate, PairPredicates, pair_filter,
                  pair_filters, PairPredicate *, PairPredicates,
                  obj->set_was_used(true);
                  , , );
  /**@}*/
  ParticleIndexPairs get_indexes() const;
  ParticleIndexPairs get_range_indexes() const;
  ModelObjectsTemp do_get_inputs() const;
  void do_apply(const PairModifier *sm) const;
  ParticleIndexes get_all_possible_indexes() const;

 private:
  virtual std::size_t do_get_contents_hash() const IMP_OVERRIDE;
#endif
  IMP_OBJECT_METHODS(CloseBipartitePairContainer);
};

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CLOSE_BIPARTITE_PAIR_CONTAINER_H */

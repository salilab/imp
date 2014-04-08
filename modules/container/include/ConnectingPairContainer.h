/**
 *  \file IMP/container/ConnectingPairContainer.h
 *  \brief A container which has pairs which ensure a set is connected
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CONNECTING_PAIR_CONTAINER_H
#define IMPCONTAINER_CONNECTING_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/kernel/internal/ListLikeContainer.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/macros.h>
#include <IMP/OptimizerState.h>
#include <IMP/ScoreState.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! A container which keeps a set of pairs that connect a set of spheres.
/** This container is for implementing connectivity when
    - there is no ambiguity about the set of particles being connected
    - the distance score used in the restraint is a monotonic function of the
    distance between the two particles.

    The contents of this container are a set of pairs defining a tree over
    the spheres in the supplied SingletonContainer. This set of edges has
    the property that
    - for particles whose balls do not intersect, if there is a path
    connecting those balls entirely contained in the union of the set of
    all balls, then there is no edge connecting those two particles.


    It is significantly more efficient than the ConnectivityRestraint
    but less flexible.
    \see {core::ConnectivityRestraint}
 */
class IMPCONTAINEREXPORT ConnectingPairContainer :
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public PairContainer
#else
    public kernel::internal::ListLikeContainer<kernel::PairContainer>
#endif
    {
  IMP::base::PointerMember<kernel::SingletonContainer> sc_;
  IMP::base::PointerMember<core::internal::MovedSingletonContainer> mv_;
  base::PointerMember<ScoreState> score_state_;
  kernel::ParticlePairsTemp data_;
  double error_bound_;
  bool mst_;
  void initialize(SingletonContainer *sc);

 public:
  /** For efficiency, the set of edges is only updated occasionally. The
   error parameter determines how far particles need to move before
   the set of edges is updated.*/
  ConnectingPairContainer(kernel::SingletonContainer *sc, double error);

#if defined(IMP_DOXYGEN) || defined(SWIG)
  kernel::ParticleIndexPairs get_indexes() const;
  kernel::ParticleIndexPairs get_range_indexes() const;
  kernel::ModelObjectsTemp do_get_inputs() const;
  void do_apply(const PairModifier *sm) const;
  kernel::ParticleIndexes get_all_possible_indexes() const;
  IMP_OBJECT_METHODS(ConnectingPairContainer);

 private:
  virtual std::size_t do_get_contents_hash() const IMP_OVERRIDE;
#else
  virtual kernel::ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  kernel::ModelObjectsTemp get_score_state_inputs() const;
  void do_score_state_before_evaluate();
  void do_score_state_after_evaluate() {}
  virtual kernel::ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConnectingPairContainer);
  bool get_is_decomposable() const { return false; }
#endif
};

IMP_OBJECTS(ConnectingPairContainer, ConnectingPairContainers);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CONNECTING_PAIR_CONTAINER_H */

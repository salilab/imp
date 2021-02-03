/**
 *  \file container/include/internal/CloseBipartitePairContainer.h
 *  \brief Internal class of close bipartite pair container
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_INTERNAL_CONTAINER_CLOSE_BIPARTITE_PAIR_CONTAINER_H
#define IMPCONTAINER_INTERNAL_CONTAINER_CLOSE_BIPARTITE_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/core/ClosePairsFinder.h>
#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/PairContainer.h>
#include <IMP/PairPredicate.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/ContainerScoreState.h>
#include <IMP/internal/ListLikeContainer.h>
#include <boost/unordered_map.hpp>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

/** \brief Return all close unordered pairs of particles taken from
    the SingletonContainer

    See ClosePairContainer for a more detailed description. This
    container lists all close pairs of particles where one particle is
    taken from each of the input sets.

    \note Any passed ClosePairsFinder is ignored.

    \usesconstraint
 */
class IMPCONTAINEREXPORT CloseBipartitePairContainer
    : public IMP::internal::ListLikeContainer<PairContainer> {
  typedef IMP::internal::ListLikeContainer<PairContainer> P;
  IMP::PointerMember<SingletonContainer> sc_[2];
  bool were_close_, reset_;
  ObjectKey key_;
  // moved stuff
  ParticleIndexes rbs_[2];
  ParticleIndexes xyzrs_[2];
  boost::unordered_map<ParticleIndex, ParticleIndexes>
      constituents_;
  double slack_, distance_;
  algebra::Sphere3Ds rbs_backup_sphere_[2];
  algebra::Rotation3Ds rbs_backup_rot_[2];
  algebra::Sphere3Ds xyzrs_backup_[2];
  ParticleIndex covers_[2];
  typedef IMP::internal::ContainerScoreState<CloseBipartitePairContainer>
      SS;
  PointerMember<SS> score_state_;
  void initialize(SingletonContainer *a, SingletonContainer *b,
                  ParticleIndex cover_a, ParticleIndex cover_b,
                  double distance, double slack, ObjectKey key);

 public:
  ModelObjectsTemp get_score_state_inputs() const;
  //! Get the individual particles from the passed SingletonContainer
  CloseBipartitePairContainer(SingletonContainer *a, SingletonContainer *b,
                                  double distance, double slack = 1,
                                  std::string name =
                                      "CoreBipartiteClosePairContainer%1%");

  //! make sure you know what you are doing
  CloseBipartitePairContainer(SingletonContainer *a, SingletonContainer *b,
                                  ParticleIndex cover_a,
                                  ParticleIndex cover_b, ObjectKey key,
                                  double distance, double slack = 1,
                                  std::string name =
                                      "CloseBipartitePairContainer%1%");

  /** @name Methods to control the set of filters

     PairPredicate objects can be added as filters on the
     containers. Pairs that do not satisfy (evaluate to zero) any
     of the PairPredicate are excluded from the close pairs list.
  */
  /**@{*/
  IMP_LIST_ACTION(public, PairFilter, PairFilters, pair_filter, pair_filters,
                  PairPredicate *, PairPredicates,
  {
    set_has_dependencies(false);
    score_state_->set_has_dependencies(false);
    obj->set_was_used(true);
  },
  {
    score_state_->set_has_dependencies(false);
    set_has_dependencies(false);
  }, );
  /**@}*/
  void clear_caches() { reset_ = true; }
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  void do_score_state_before_evaluate();
  void do_score_state_after_evaluate() {}
  virtual ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(CloseBipartitePairContainer);
};

IMP_OBJECTS(CloseBipartitePairContainer, CloseBipartitePairContainers);

IMPCONTAINER_END_INTERNAL_NAMESPACE

#endif /* IMPCONTAINER_INTERNAL_CONTAINER_CLOSE_BIPARTITE_PAIR_CONTAINER_H */

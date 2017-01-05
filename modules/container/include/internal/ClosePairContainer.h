/**
 *  \file container/internal/ClosePairContainer.h
 *  \brief Internal class of close pair container
 *
 *  Copyright 2007-2017 IMP Inventors. Close rights reserved.
 */

#ifndef IMPCONTAINER_INTERNAL_CONTAINER_CLOSE_PAIR_CONTAINER_H
#define IMPCONTAINER_INTERNAL_CONTAINER_CLOSE_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/core/ClosePairsFinder.h>
#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairPredicate.h>
#include <IMP/generic.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/ContainerScoreState.h>
#include <IMP/internal/ListLikeContainer.h>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

class IMPCONTAINEREXPORT ClosePairContainer
    : public IMP::internal::ListLikeContainer<PairContainer> {
  IMP::PointerMember<SingletonContainer> c_;
  IMP::PointerMember<core::ClosePairsFinder> cpf_;
  IMP::PointerMember<core::internal::MovedSingletonContainer> moved_;
  unsigned int moved_count_;
  bool first_call_;
  double distance_, slack_;
  unsigned int updates_, rebuilds_, partial_rebuilds_;
  typedef IMP::internal::ContainerScoreState<ClosePairContainer> SS;
  PointerMember<SS> score_state_;

  void initialize(SingletonContainer *c, double distance, double slack,
                  core::ClosePairsFinder *cpf);

  void check_duplicates_input() const;
  void check_list(bool include_slack) const;
  void do_first_call();
  void do_incremental();
  void do_rebuild();

 public:
  ModelObjectsTemp get_score_state_inputs() const;
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  void do_score_state_before_evaluate();
  void do_score_state_after_evaluate() {}

  ClosePairContainer(SingletonContainer *c, double distance,
                     core::ClosePairsFinder *cpf, double slack = 1,
                         std::string name = "ClosePairContainer%1%");

  IMP_LIST_ACTION(public, PairFilter, PairFilters, pair_filter, pair_filters,
                  PairPredicate *, PairPredicates,
  {
    set_has_dependencies(false);
    obj->set_was_used(true);
  },
  { set_has_dependencies(false); }, );

  void clear_caches() { first_call_ = true; }
  double get_slack() const { return slack_; }
  double get_distance() const { return distance_; }
  void update() { do_score_state_before_evaluate(); }
  SingletonContainer *get_singleton_container() const { return c_; }
  core::ClosePairsFinder *get_close_pairs_finder() const { return cpf_; }
  void set_slack(double d);
  Restraints create_decomposition(PairScore *ps) const {
    ParticleIndexPairs all = get_range_indexes();
    Restraints ret(all.size());
    for (unsigned int i = 0; i < all.size(); ++i) {
      ret[i] = new core::PairRestraint(get_model(), ps, all[i]);
    }
    return ret;
  }
  template <class PS>
  Restraints create_decomposition_t(PS *ps) const {
    ParticleIndexPairs all = get_range_indexes();
    Restraints ret(all.size());
    for (unsigned int i = 0; i < all.size(); ++i) {
      ret[i] = IMP::create_restraint(
          ps, IMP::internal::get_particle(get_model(), all[i]));
    }
    return ret;
  }
  unsigned int get_number_of_update_calls() const { return updates_; }
  unsigned int get_number_of_full_rebuilds() const { return rebuilds_; }
  unsigned int get_number_of_partial_rebuilds() const {
    return partial_rebuilds_;
  }

  IMP_OBJECT_METHODS(ClosePairContainer);
};

IMP_OBJECTS(ClosePairContainer, ClosePairContainers);

IMPCONTAINER_END_INTERNAL_NAMESPACE

#endif /* IMPCONTAINER_INTERNAL_CONTAINER_CLOSE_PAIR_CONTAINER_H */

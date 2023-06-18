/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_RESTRAINT_H
#define IMPKERNEL_CONTAINER_RESTRAINT_H

#include <IMP/kernel_config.h>
#include "../base_types.h"
#include "../Restraint.h"
#include "create_decomposition.h"
#include "AccumulatorScoreModifier.h"
#include "functors.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
/** When programming in C++, you can use ContainerRestraint instead
    of a SingletonsRestraint, PairsRestraint, etc. The result is
    somewhat faster (20% or so).
*/
template <class Score, class Container>
class ContainerRestraint : public Restraint {
  IMP::PointerMember<Container> pc_;
  IMP::Pointer<Score> ss_;
  IMP::Pointer<AccumulatorScoreModifier<Score, Container> > acc_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), pc_, ss_);

    // recreate the AccumulatorScoreModifier
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      acc_ = create_accumulator_score_modifier(ss_.get(), pc_.get());
    }
  }

 public:
  ContainerRestraint(Score *ss, Container *pc,
                     std::string name = "GroupnamesRestraint %1%");

  ContainerRestraint() {}

 public:
  void do_add_score_and_derivatives(IMP::ScoreAccumulator sa) const
      override;

  void do_add_score_and_derivatives_moved(IMP::ScoreAccumulator sa,
                const ParticleIndexes &moved_pis,
                const ParticleIndexes &reset_pis) const override;

  ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(ContainerRestraint);
  ;

  //! Get the container used to store Particles
  typename Container::ContainedTypes get_arguments() const {
    return pc_->get();
  }

  Score *get_score() const { return acc_->get_score_object(); }

  Restraints do_create_decomposition() const override;
  Restraints do_create_current_decomposition() const override;

  IMP_IMPLEMENT(double get_last_score() const override;);
};

/** Helper to create a ContainerRestraint without specifying the types. Make
    sure the score and container passed have their real type, not Container
    or PairScore.
    See ContainerRestraint
*/
template <class Score, class Container>
inline Restraint *create_container_restraint(Score *s, Container *c,
                                             std::string name = std::string()) {
  if (name == std::string()) {
    name = s->get_name() + " and " + c->get_name();
  }
  return new ContainerRestraint<Score, Container>(s, c, name);
}

template <class Score, class C>
ContainerRestraint<Score, C>::ContainerRestraint(Score *ss, C *pc,
                                                 std::string name)
    : Restraint(pc->get_model(), name),
      pc_(pc),
      ss_(ss),
      acc_(create_accumulator_score_modifier(ss, pc)) {}

template <class Score, class C>
double ContainerRestraint<Score, C>::get_last_score() const {
  return acc_->get_score();
}

template <class Score, class C>
void ContainerRestraint<Score, C>::do_add_score_and_derivatives(
    ScoreAccumulator accum) const {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(acc_);
  IMP_CHECK_OBJECT(pc_);
  acc_->set_accumulator(accum);
  pc_->apply_generic(acc_.get());
}

template <class Score, class C>
void ContainerRestraint<Score, C>::do_add_score_and_derivatives_moved(
    ScoreAccumulator accum,
    const ParticleIndexes &moved_pis,
    const ParticleIndexes &reset_pis) const {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(acc_);
  IMP_CHECK_OBJECT(pc_);
  acc_->set_accumulator(accum);
  acc_->set_container(pc_);
  pc_->apply_generic_moved(acc_.get(), moved_pis, reset_pis);
}

template <class Score, class C>
ModelObjectsTemp ContainerRestraint<Score, C>::do_get_inputs() const {
  IMP_OBJECT_LOG;
  ModelObjectsTemp ret;
  ret += acc_->get_score_object()->get_inputs(get_model(),
                                              pc_->get_all_possible_indexes());
  ret.push_back(pc_);
  return ret;
}

template <class Score, class C>
Restraints ContainerRestraint<Score, C>::do_create_decomposition() const {
  return IMP::internal::create_decomposition(
      get_model(), acc_->get_score_object(), pc_.get(), get_name());
}

template <class Score, class C>
Restraints ContainerRestraint<Score, C>::do_create_current_decomposition()
    const {
  if (get_last_score() == 0) return Restraints();
  return IMP::internal::create_current_decomposition(
      get_model(), acc_->get_score_object(), pc_.get(), get_name());
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_CONTAINER_RESTRAINT_H */

/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_RESTRAINT_H
#define IMPKERNEL_CONTAINER_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>
#include "../base_types.h"
#include "../Restraint.h"
#include "create_decomposition.h"
#include "../restraint_macros.h"
#include "AccumulatorScoreModifier.h"
#include "functors.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
/** When programming in C++, you can use CoreRestraint instead
    of a SingletonsRestraint, PairsRestraint, etc. The result is
    somewhat faster (20% or so).
*/
template <class Score, class Container>
class ContainerRestraint : public Restraint
{
  IMP::base::OwnerPointer<Container> pc_;
  IMP::base::Pointer<AccumulatorScoreModifier<Score> > acc_;
public:
  ContainerRestraint(Score *ss,
                     Container *pc,
                     std::string name="GroupnamesRestraint %1%");

  public:
  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa)
    const IMP_OVERRIDE;
  IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ContainerRestraint);;

  //! Get the container used to store Particles
  typename Container::ContainedTypes
  get_arguments() const {
    return pc_->get();
  }

  Score* get_score() const {
    return acc_->get_score_object();
  }

  Restraints do_create_decomposition() const;
  Restraints do_create_current_decomposition() const;

  IMP_IMPLEMENT(double get_last_score() const;);
};



/** Helper to create a ContainerRestraint without specifying the types. Make
    sure the score and container passed have their real type, not Container
    or PairScore.
    \relatesalso ContainerRestraint
*/
template <class Score, class Container>
inline Restraint *create_container_restraint(Score *s, Container*c,
                            std::string name=std::string()) {
  if (name==std::string()) {
    name= s->get_name()+ " and "+c->get_name();
  }
  return new ContainerRestraint<Score, Container>(s, c, name);
}



template <class Score, class C>
ContainerRestraint<Score, C>
::ContainerRestraint(Score *ss,
                     C *pc,
                     std::string name):
  Restraint(pc->get_model(), name),
  pc_(pc),
  acc_(create_accumulator_score_modifier(ss)) {

}

template <class Score, class C>
double ContainerRestraint<Score, C>
::get_last_score() const {
  return acc_->get_score();
}

template <class Score, class C>
void ContainerRestraint<Score, C>
::do_add_score_and_derivatives(ScoreAccumulator accum) const
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(acc_);
  IMP_CHECK_OBJECT(pc_);
  acc_->set_accumulator(accum);
  pc_->apply_generic(acc_.get());
}


template <class Score, class C>
ModelObjectsTemp ContainerRestraint<Score, C>::do_get_inputs() const
{
  IMP_OBJECT_LOG;
  ModelObjectsTemp ret;
  ret+= acc_->get_score_object()->get_inputs(get_model(),
                                      pc_->get_all_possible_indexes());
  ret.push_back(pc_);
  return ret;
}

template <class Score, class C>
Restraints ContainerRestraint<Score, C>::do_create_decomposition() const {
  return IMP::kernel::internal::create_decomposition(get_model(),
                                             acc_->get_score_object(),
                                             pc_.get(),
                                             get_name());
}

template <class Score, class C>
Restraints
ContainerRestraint<Score, C>::do_create_current_decomposition() const {
  if (get_last_score()==0) return Restraints();
  return IMP::kernel::internal::create_current_decomposition(get_model(),
                                                     acc_->get_score_object(),
                                                     pc_.get(),
                                                     get_name());
}


IMPKERNEL_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_CONTAINER_RESTRAINT_H */

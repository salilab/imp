/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H
#define IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H

#include <IMP/kernel/kernel_config.h>
#include "../ScoringFunction.h"
#include "../Model.h"
#include <IMP/base/object_macros.h>
#include "restraint_evaluation.h"
#include "RestraintsScoringFunction.h"

#include "evaluate_utility.h"
#include "utility.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

inline Model *get_model( Restraint* rs) {
  IMP_CHECK_OBJECT(rs);
  IMP_USAGE_CHECK(rs,
                  "Can't null restraint as we"
                  << " need to find the model.");
  Model *m= rs->get_model();
  IMP_USAGE_CHECK(m, "Model was not set on restraint "
                  << Showable(rs));
  return m;
}


inline Model *get_model(const RestraintsTemp &rs) {
  IMP_USAGE_CHECK(!rs.empty(),
                  "Can't pass empty list of restraints as we"
                  << " need to find the model.");
  Model *m= rs[0]->get_model();
  IMP_USAGE_CHECK(m, "Model was not set on restraint "
                  << Showable(rs[0]));
  return m;
}







/** Implement a scoring function on a single restraint.
 */
template <class RestraintType>
class RestraintScoringFunction: public ScoringFunction {
  base::OwnerPointer<RestraintType> r_;
 public:
  RestraintScoringFunction(RestraintType* r):
    ScoringFunction(IMP::kernel::internal::get_model(r),
                    r->get_name()+"SF"), r_(r){}
  RestraintScoringFunction(RestraintType* r,
                           std::string name):
    ScoringFunction(IMP::kernel::internal::get_model(r), name), r_(r){}
  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa,
                                    const ScoreStatesTemp &ss) IMP_OVERRIDE;
  Restraints create_restraints() const IMP_OVERRIDE;
  ScoreStatesTemp get_required_score_states() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RestraintScoringFunction);
};

template <class RestraintType>
void
RestraintScoringFunction<RestraintType>
::do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa,
                               const ScoreStatesTemp &ss) {
  internal::protected_evaluate(sa, r_.get(), ss, get_model());
}

template <class RestraintType>
Restraints RestraintScoringFunction<RestraintType>::create_restraints() const {
  return Restraints(1, r_);
}


template <class RestraintType>
ScoreStatesTemp
RestraintScoringFunction<RestraintType>
::get_required_score_states() const {
  return get_model()->get_required_score_states(r_);
}




/** Implement a scoring function on a single restraint.
 */
template <class RestraintType>
class WrappedRestraintScoringFunction: public ScoringFunction {
  base::OwnerPointer<RestraintType> r_;
  double weight_;
  double max_;
 public:
  WrappedRestraintScoringFunction(RestraintType* r,
                                  double weight,
                                  double max):
    ScoringFunction(IMP::kernel::internal::get_model(r),
                    r->get_name()+"SF"), r_(r),
      weight_(weight), max_(max){}
  WrappedRestraintScoringFunction(RestraintType* r,
                                  double weight,
                                  double max,
                                  std::string name):
    ScoringFunction(IMP::kernel::internal::get_model(r),
                      name), r_(r),
      weight_(weight), max_(max){}
  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa,
                                    const ScoreStatesTemp &ss) IMP_OVERRIDE;
  Restraints create_restraints() const IMP_OVERRIDE;
  ScoreStatesTemp get_required_score_states() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WrappedRestraintScoringFunction);
};

template <class RestraintType>
void
WrappedRestraintScoringFunction<RestraintType>
::do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa,
                               const ScoreStatesTemp &ss) {
  IMP::kernel::ScoreAccumulator msa(sa, weight_, max_);
  protected_evaluate(msa, r_.get(), ss, get_model());
}


template <class RestraintType>
Restraints
WrappedRestraintScoringFunction<RestraintType>::create_restraints() const {
  IMP_NEW(RestraintSet, rs, (get_name()+" weights"));
  rs->add_restraint(r_);
  rs->set_model(get_model());
  rs->set_maximum_score(max_);
  rs->set_weight(weight_);
  return Restraints(1, rs);
}

template <class RestraintType>
ScoreStatesTemp
WrappedRestraintScoringFunction<RestraintType>
::get_required_score_states() const {
  ScoreStatesTemp ret= get_model()->get_required_score_states(r_);
  return ret;
}


/** Create a ScoringFunction on a single restraints.*/
template <class RestraintType>
inline ScoringFunction* create_scoring_function(RestraintType* rs,
                                               double weight=1.0,
                                                double max=NO_MAX,
                                                std::string name=
                                                std::string()) {
  if (name.empty()) {
    name= rs->get_name()+" SF";
  }
  if (weight==1.0 && max==NO_MAX) {
    return new internal::RestraintScoringFunction<RestraintType>(rs,
                                                                 name);
  } else {
    return new internal::WrappedRestraintScoringFunction<RestraintType>(rs,
                                                                        weight,
                                                                        max,
                                                                        name);
  }
}


IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H */

/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H
#define IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H

#include "../kernel_config.h"
#include "../ScoringFunction.h"
#include "../Model.h"
#include "../scoring_function_macros.h"
#include "restraint_evaluation.h"
#include "RestraintsScoringFunction.h"

#include "evaluate_utility.h"
#include "utility.h"

IMP_BEGIN_INTERNAL_NAMESPACE

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
    ScoringFunction(IMP::internal::get_model(r),
                    r->get_name()+"SF"), r_(r){}
  RestraintScoringFunction(RestraintType* r,
                           std::string name):
    ScoringFunction(IMP::internal::get_model(r), name), r_(r){}
  IMP_SCORING_FUNCTION(RestraintScoringFunction);
};

template <class RestraintType>
std::pair<double, bool>
RestraintScoringFunction<RestraintType>
::do_evaluate(bool derivatives,
              const ScoreStatesTemp &ss) {
  double weight=r_->get_weight();
  IMP_SF_CALL_PROTECTED_EVALUATE(false, false, derivatives,
                                 unprotected_evaluate(r_.get(),
                                                      r_->get_maximum_score(),
                                                      weight,
                                                      get_model()),
                                 weight, ss);
  return ret;
}
template <class RestraintType>
std::pair<double, bool>
RestraintScoringFunction<RestraintType>
::do_evaluate_if_good(bool derivatives,
                      const ScoreStatesTemp &ss) {
  double weight=r_->get_weight();
  IMP_SF_CALL_PROTECTED_EVALUATE(true, false, derivatives,
                                 unprotected_evaluate(r_.get(),
                                                      r_->get_maximum_score(),
                                                      weight,
                                                      get_model()),
                                 weight, ss);
  return ret;

}

template <class RestraintType>
std::pair<double, bool>
RestraintScoringFunction<RestraintType>
::do_evaluate_if_below(bool derivatives,
                       double max,
                       const ScoreStatesTemp &ss) {
  double weight=r_->get_weight();
  IMP_SF_CALL_PROTECTED_EVALUATE(false, true, derivatives,
                                 unprotected_evaluate(r_.get(),
                                                      max/weight,
                                                      weight,
                                                      get_model()),
                                 weight, ss);
  return ret;

}
template <class RestraintType>
Restraints RestraintScoringFunction<RestraintType>::create_restraints() const {
  return Restraints(1, r_);
}

template <class RestraintType>
void RestraintScoringFunction<RestraintType>::do_show(std::ostream &out) const {
  IMP_UNUSED(out);
}


template <class RestraintType>
ScoreStatesTemp
RestraintScoringFunction<RestraintType>
::get_required_score_states(const DependencyGraph &dg,
                            const DependencyGraphVertexIndex &index) const {
  return IMP::get_required_score_states(RestraintsTemp(1,r_), dg, index);
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
    ScoringFunction(IMP::internal::get_model(r),
                    r->get_name()+"SF"), r_(r),
      weight_(weight), max_(max){}
  WrappedRestraintScoringFunction(RestraintType* r,
                                  double weight,
                                  double max,
                                  std::string name):
    ScoringFunction(IMP::internal::get_model(r),
                      name), r_(r),
      weight_(weight), max_(max){}
  IMP_SCORING_FUNCTION(WrappedRestraintScoringFunction);
};

template <class RestraintType>
std::pair<double, bool>
WrappedRestraintScoringFunction<RestraintType>
::do_evaluate(bool derivatives,
              const ScoreStatesTemp &ss) {
  double weight=r_->get_weight()*weight_;
  double max= std::min(max_/weight, r_->get_maximum_score());
  IMP_SF_CALL_PROTECTED_EVALUATE(false, false, derivatives,
                                 unprotected_evaluate(r_.get(), max,
                                                      weight,
                                                      get_model()),
                                 weight, ss);
  return ret;
}

template <class RestraintType>
std::pair<double, bool>
WrappedRestraintScoringFunction<RestraintType>
::do_evaluate_if_good(bool derivatives,
                      const ScoreStatesTemp &ss) {
  double weight=r_->get_weight()*weight_;
  double max= std::min(max_/r_->get_weight(), r_->get_maximum_score());
  IMP_SF_CALL_PROTECTED_EVALUATE(true, false, derivatives,
                                 unprotected_evaluate(r_.get(),
                                                      max,
                                                      weight,
                                                      get_model()),
                                 weight, ss);
  return ret;
}

template <class RestraintType>
std::pair<double, bool>
WrappedRestraintScoringFunction<RestraintType>
::do_evaluate_if_below(bool derivatives,
                       double imax,
                       const ScoreStatesTemp &ss) {
  double weight=r_->get_weight()*weight_;
  double max= std::min(max_/r_->get_weight(), imax/weight);
  max= std::min(max, r_->get_maximum_score());
  IMP_SF_CALL_PROTECTED_EVALUATE(false, true, derivatives,
                                 unprotected_evaluate(r_.get(), max,
                                                      weight, get_model()),
                                 weight, ss);
  return ret;
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
::get_required_score_states(const DependencyGraph &dg,
                            const DependencyGraphVertexIndex &index) const {
  return IMP::get_required_score_states(RestraintsTemp(1,r_), dg, index);
}

template <class RestraintType>
void
WrappedRestraintScoringFunction<RestraintType>::do_show(std::ostream &out)
    const {
  IMP_UNUSED(out);
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


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_SCORING_FUNCTIONS_H */

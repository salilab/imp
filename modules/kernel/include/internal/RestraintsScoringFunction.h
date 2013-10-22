/**
 *  \file IMP/declare_ScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H
#define IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H

#include <IMP/kernel/kernel_config.h>
#include "../ScoringFunction.h"
#include "../scoring_function_macros.h"
#include "../container_macros.h"
#include "restraint_evaluation.h"
#include "container_helpers.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

/** Create a scoring function on a list of restraints.

    The templated storage is so that non-ref counted pointers can
    be used when creating the cache function in Restraints.
*/
template <class Storage>
class GenericRestraintsScoringFunction : public ScoringFunction {
  double weight_;
  double max_;
  Storage restraints_;

 protected:
  GenericRestraintsScoringFunction(kernel::Model *m, double weight = 1.0,
                                   double max = NO_MAX,
                                   std::string name =
                                       "RestraintsScoringFunction%1%")
      : ScoringFunction(m, name), weight_(weight), max_(max) {}

 public:
  GenericRestraintsScoringFunction(const RestraintsTemp &rs,
                                   double weight = 1.0, double max = NO_MAX,
                                   std::string name =
                                       "RestraintsScoringFunction%1%")
      : ScoringFunction(IMP::kernel::internal::get_model(rs), name),
        weight_(weight),
        max_(max),
        restraints_(rs.begin(), rs.end()) {}

  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa,
                                    const ScoreStatesTemp &ss) IMP_OVERRIDE {
    IMP_OBJECT_LOG;
    protected_evaluate(sa, restraints_, ss, get_model());
  }
  Restraints create_restraints() const IMP_OVERRIDE {
    IMP_OBJECT_LOG;
    IMP_NEW(RestraintSet, rs, (get_model(), weight_, get_name() + " wrapper"));
    rs->set_maximum_score(max_);
    rs->add_restraints(restraints_);
    return Restraints(1, rs);
  }
  ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE { return restraints_; }
  const Storage &get_restraints() const { return restraints_; }
  void set_restraints(const RestraintsTemp &s) {
    set_has_dependencies(false);
    restraints_ = s;
  }
  IMP_OBJECT_METHODS(GenericRestraintsScoringFunction);
};

typedef GenericRestraintsScoringFunction<Restraints> RestraintsScoringFunction;
IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_RESTRAINTS_SCORING_FUNCTION_H */

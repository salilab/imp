/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/ScoringFunction.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/internal/evaluate_utility.h"
#include "IMP/kernel/internal/scoring_functions.h"
#include "IMP/kernel/scoring_function_macros.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/generic.h"
#include "IMP/kernel/utility.h"



IMPKERNEL_BEGIN_NAMESPACE

// in namespace so it can be made a friend.
class NullScoringFunction: public ScoringFunction {
public:
  NullScoringFunction() {}
  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator ,
                                    const ScoreStatesTemp &) IMP_OVERRIDE {
  }
  Restraints create_restraints() const IMP_OVERRIDE {
    return Restraints();
  }
  ScoreStatesTemp get_required_score_states() const IMP_OVERRIDE {
    return ScoreStatesTemp();
  }
  IMP_OBJECT_METHODS(NullScoringFunction);
};

ScoringFunction::ScoringFunction(Model *m,
                                 std::string name): ModelObject(m, name){
}

double ScoringFunction::evaluate_if_good(bool derivatives) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  ensure_dependencies();
  es_.score=0;
  es_.good=true;
  const ScoreAccumulator sa=get_score_accumulator_if_good(derivatives);
  do_add_score_and_derivatives(sa, ss_);
  return es_.score;
}

double ScoringFunction::evaluate(bool derivatives) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  ensure_dependencies();
  es_.score=0;
  es_.good=true;
  const ScoreAccumulator sa=get_score_accumulator(derivatives);
  do_add_score_and_derivatives(sa, ss_);
  return es_.score;
}

double ScoringFunction::evaluate_if_below(bool derivatives, double max) {
  IMP_OBJECT_LOG;
  set_was_used(true);
  ensure_dependencies();
  es_.score=0;
  es_.good=true;
  const ScoreAccumulator sa=get_score_accumulator_if_below(derivatives,
                                                     max);
  do_add_score_and_derivatives(sa, ss_);
  return es_.score;
}
void
ScoringFunction::do_update_dependencies() {
  IMP_OBJECT_LOG;
  // can't check here as create_restraints can cause a loop
  // but we must make sure they are ordered
  ScoreStatesTemp ret= get_required_score_states();

  ModelObjectsTemp ops= get_model()->get_optimized_particles();
  for (unsigned int i=0; i< ops.size(); ++i) {
    ret+=get_model()->get_required_score_states(ops[i]);
  }
  ss_= get_update_order(ret);
}


void
ScoringFunction::clear_caches() {
  get_model()->clear_caches();
}

ScoringFunction*
ScoringFunctionAdaptor::get(const RestraintsTemp &sf) {
  if (!sf.empty()) {
    return new internal::RestraintsScoringFunction(sf);
  } else {
    return new NullScoringFunction();
  }}

ScoringFunction*
ScoringFunctionAdaptor::get(const Restraints&sf) {
  if (!sf.empty()) {
    return new internal::RestraintsScoringFunction(sf);
  } else {
    return new NullScoringFunction();
  }
}
ScoringFunction* ScoringFunctionAdaptor::get(Model *sf) {
  return sf->create_scoring_function();
}
ScoringFunction* ScoringFunctionAdaptor::get(Restraint *sf) {
  return sf->create_scoring_function();
}

namespace {
  unsigned int sf_num_children(Restraint*r) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
    if (rs) return rs->get_number_of_restraints();
    else return 0;
  }
}


void show_restraint_hierarchy(ScoringFunctionAdaptor r, std::ostream &out) {
  Restraints cur= r->create_restraints();
  for (unsigned int ii=0; ii< cur.size(); ++ii) {
      Restraint*curr= cur[ii];
      RestraintSet *rs=dynamic_cast<RestraintSet*>(curr);
       if (!rs) {
         IMP_PRINT_TREE(out, Restraint*, curr, 0,
                        dynamic_cast<RestraintSet*>(n)->get_restraint,
                        out << Showable(n)
                        << " " << n->get_maximum_score() << " "
                        << n->get_weight() );
       } else {
         IMP_PRINT_TREE(out, Restraint*, rs, sf_num_children(n),
                        dynamic_cast<RestraintSet*>(n)->get_restraint,
                        out << Showable(n)
                        << " " << n->get_maximum_score() << " "
                        << n->get_weight() );
       }
  }
}

namespace {
ScoringFunctions create_decomposition(Restraint *r, double w, double max) {
  if (!r) return ScoringFunctions();
  RestraintSet* rs= dynamic_cast<RestraintSet*>(r);
  if (rs) {
    ScoringFunctions ret;
    for (RestraintSet::RestraintIterator it=rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      ret= ret+create_decomposition(*it, w* rs->get_weight(),
                               std::min(max, rs->get_maximum_score()));
    }
    return ret;
  } else {
    return ScoringFunctions(1, r->create_scoring_function(w, max));
  }
}

ScoringFunctions
create_decomposition_into_scoring_functions(const RestraintsTemp &sf) {
  ScoringFunctions ret;
  for (unsigned int i=0; i< sf.size(); ++i) {
    base::Pointer<Restraint> r= sf[i]->create_decomposition();
    ret= ret+ create_decomposition(r, 1.0, NO_MAX);
  }
  return ret;
}


}

ScoringFunctions create_decomposition(ScoringFunction *sf) {
  ScoringFunctions ret;
  ret= create_decomposition_into_scoring_functions(sf->create_restraints());
  return ret;
}
IMPKERNEL_END_NAMESPACE

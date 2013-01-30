/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/RestraintsScoringFunction.h"
#include "IMP/internal/scoring_functions.h"
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE


RestraintsScoringFunction::RestraintsScoringFunction(const RestraintsTemp &r,
                                                     double weight,
                                                     double max,
                                                     std::string name):
  ScoringFunction(IMP::internal::get_model(r), name),
    weight_(weight), max_(max){
  set_restraints(r);
}


RestraintsScoringFunction::RestraintsScoringFunction(Model *m,
                                                     double weight,
                                                     double max,
                                                     std::string name):
  ScoringFunction(m, name),
  weight_(weight), max_(max){
}


void
RestraintsScoringFunction
::do_add_score_and_derivatives(IMP::ScoreAccumulator sa,
                               const ScoreStatesTemp &ss) {
  protected_evaluate(sa, get_restraints(), ss, get_model());
}


Restraints RestraintsScoringFunction::create_restraints() const {
  IMP_NEW(RestraintSet, rs, (get_name()+" wrapper"));
  rs->set_model(get_model());
  rs->set_maximum_score(max_);
  rs->set_weight(weight_);
  rs->add_restraints(get_restraints());
  return Restraints(1, rs);
}

ScoreStatesTemp
RestraintsScoringFunction::get_required_score_states() const {
  ScoreStatesTemp ret;
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    ret+=get_model()->get_required_score_states(get_restraint(i));
  }
  return ret;
}


IMP_LIST_IMPL(RestraintsScoringFunction, Restraint, restraint, Restraint*,
              Restraints);

IMP_END_INTERNAL_NAMESPACE

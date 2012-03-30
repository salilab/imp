/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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


void RestraintsScoringFunction::set_restraints(const RestraintsTemp &r) {
  rs_.clear();
  rss_.clear();
  for (unsigned int i=0; i< r.size(); ++i) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r[i].get());
    if (rs) {
      rss_.push_back(rs);
    } else {
      rs_.push_back(r[i]);
    }
  }
}

std::pair<double, bool> RestraintsScoringFunction::do_evaluate(bool derivatives,
                                             const ScoreStatesTemp &ss) {
  IMP_SF_CALL_PROTECTED_EVALUATE(false, false, derivatives,
                                 unprotected_evaluate(rs_, rss_, max_/weight_,
                                                      weight_, get_model()),
                                 weight_, ss);
  return ret;
}
std::pair<double, bool>
RestraintsScoringFunction::do_evaluate_if_good(bool derivatives,
                                        const ScoreStatesTemp &ss) {
  IMP_SF_CALL_PROTECTED_EVALUATE(true, false, derivatives,
                                 unprotected_evaluate(rs_, rss_, max_/weight_,
                                                      weight_, get_model()),
                                 weight_, ss);
  return ret;
}

std::pair<double, bool>
RestraintsScoringFunction::do_evaluate_if_below(bool derivatives,
                                         double max,
                                         const ScoreStatesTemp &ss) {
  IMP_SF_CALL_PROTECTED_EVALUATE(false, true, derivatives,
                                 unprotected_evaluate(rs_, rss_, max/weight_,
                                                      weight_, get_model()),
                                 weight_, ss);
  return ret;
}


Restraints RestraintsScoringFunction::create_restraints() const {
  IMP_NEW(RestraintSet, rs, (get_name()+" wrapper"));
  rs->set_model(get_model());
  rs->set_maximum_score(max_);
  rs->set_weight(weight_);
  rs->add_restraints(rs_);
  rs->add_restraints(rss_);
  return Restraints(1, rs);
}

ScoreStatesTemp
RestraintsScoringFunction::get_required_score_states(const DependencyGraph &g,
                                            const DependencyGraphVertexIndex&i)
  const {
  return IMP::get_required_score_states(static_cast<RestraintsTemp>(rs_)
                                        +static_cast<RestraintsTemp>(rss_),
                                        g, i);
}


void RestraintsScoringFunction::do_show(std::ostream &out) const {
  IMP_UNUSED(out);
}


IMP_END_INTERNAL_NAMESPACE

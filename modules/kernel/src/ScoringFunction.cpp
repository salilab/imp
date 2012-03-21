/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/ScoringFunction.h"
#include "IMP/Model.h"
#include "IMP/internal/evaluate_utility.h"
#include "IMP/internal/scoring_functions.h"
#include "IMP/internal/utility.h"
#include "IMP/generic.h"
#include "IMP/utility.h"





IMP_BEGIN_NAMESPACE

ScoringFunction::ScoringFunction(Model *m,
                                 std::string name): Tracked(this, m, name),
                                                    last_score_(-1),
                                                    last_was_good_(false){
}

ScoringFunctionInput::ScoringFunctionInput(Model *sf):
  P(IMP::internal::create_scoring_function(sf)){
}



ScoringFunctionInput::ScoringFunctionInput(const RestraintsTemp &sf):
  P(new internal::RestraintsScoringFunction(sf)){
  }
ScoringFunctionInput::ScoringFunctionInput(RestraintSet *sf):
  P(IMP::internal::create_scoring_function(sf)){}

namespace {
  unsigned int sf_num_children(Restraint*r) {
    RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
    if (rs) return rs->get_number_of_restraints();
    else return 0;
  }
}


void show_restraint_hierarchy(ScoringFunctionInput r, std::ostream &out) {
  RestraintsTemp cur= r->get_restraints();
  for (unsigned int i=0; i< cur.size(); ++i) {
      Restraint*r= cur[i];
      RestraintSet *rs=dynamic_cast<RestraintSet*>(r);
       if (!rs) {
         IMP_PRINT_TREE(out, Restraint*, r, 0,
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
ScoringFunctions create_decomposition(const RestraintsTemp &sf) {
  ScoringFunctions ret;
  for (unsigned int i=0; i< sf.size(); ++i) {
    Pointer<Restraint> r= sf[i]->create_decomposition();
    ret= ret+ create_decomposition(r, 1.0, NO_MAX);
  }
  return ret;
}

ScoringFunctions create_incremental_decomposition(const RestraintsTemp &sf) {
  ScoringFunctions ret;
  for (unsigned int i=0; i< sf.size(); ++i) {
    Pointer<Restraint> r= sf[i]->create_incremental_decomposition(1);
    ret= ret+ create_decomposition(r, 1.0, NO_MAX);
  }
  return ret;
}

}

ScoringFunctions create_decomposition(ScoringFunction *sf) {
  ScoringFunctions ret;
  ret= create_decomposition(sf->get_restraints());
  return ret;
}

ScoringFunctions create_incremental_decomposition(ScoringFunction *sf) {
  ScoringFunctions ret;
  ret= create_incremental_decomposition(sf->get_restraints());
  return ret;
}
IMP_END_NAMESPACE

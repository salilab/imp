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



#define IMP_SF_CALL_EVALUATE(ninmax, good, usemax)                      \
  std::pair<double, bool> ret;                                          \
  double inmax=ninmax;                                                  \
  if (good) inmax=std::min(inmax, max_);                                \
  if (derivatives) {                                                    \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate<true, good, usemax, true>(ss, rs_, rss_,       \
                                                 inmax,                 \
                                                 get_model());          \
    } else {                                                            \
      ret= exec_evaluate<true, good, usemax, false>(ss, rs_, rss_,      \
                                                  inmax,                \
                                                  get_model());         \
    }                                                                   \
  } else {                                                              \
    if (get_model()->get_gather_statistics()) {                         \
      ret= exec_evaluate<false, good, usemax, true>(ss, rs_, rss_,      \
                                                  inmax,                \
                                                  get_model());         \
    } else {                                                            \
      ret= exec_evaluate<false, good, usemax, false>(ss, rs_, rss_,     \
                                                   inmax,               \
                                                   get_model());        \
    }                                                                   \
  }                                                                     \
  if (!good && ret.first > inmax) {                                     \
    ret.second=false;                                                   \
  }                                                                     \
  ret.first*=weight_;                                                   \
  return ret


IMP_BEGIN_NAMESPACE

ScoringFunction::ScoringFunction(Model *m,
                                 std::string name): Tracked(this, m, name),
                                                    last_score_(-1),
                                                    last_was_good_(false){
}
const ScoreStatesTemp& ScoringFunction::get_score_states() {
  ensure_dependencies();
  return ss_;
}


namespace {

template <bool DERIV, bool GOOD, bool MAX, bool STATS, class RS, class RSS>
  inline std::pair<double, bool> exec_evaluate(const RS &restraints,
                                               const RSS &restraint_sets,
                                               double max,
                                               double initial_weight,
                                               Model *m) {
  IMP_FUNCTION_LOG;
  BOOST_STATIC_ASSERT((!GOOD || !MAX));
  double ret=0;
  double remaining=max;
  boost::scoped_ptr<boost::timer> timer;
  bool good=true;
  if (STATS) timer.reset(new boost::timer());
  for (unsigned int i=0; i< restraints.size(); ++i) {
    std::pair<double, bool> value
        =internal::do_evaluate_restraint<DERIV, GOOD, MAX, STATS>
        (restraints[i].get(),
         remaining,
         initial_weight,
         timer,
         m);
    remaining-=value.first;
    ret+=value.first;
    good= good && value.second;
  }
  for (unsigned int i=0; i< restraint_sets.size(); ++i) {
    std::pair<RestraintsTemp, RestraintSetsTemp> rsp
      = restraint_sets[i]->get_non_sets_and_sets();
    double current_max=std::min(remaining,
                                restraint_sets[i]->get_maximum_score());
    double current_weight= initial_weight
      * restraint_sets[i]->get_weight();
    std::pair<double, bool> value
        = exec_evaluate<DERIV, GOOD, MAX, STATS>(rsp.first, rsp.second,
                                                 current_max,
                                                 current_weight, m);
    remaining-=value.first;
    ret+=value.first;
    good=good&&value.second;
  }
  return std::make_pair(ret, good);
}




  template < bool DERIV, bool GOOD, bool MAX, bool STATS>
  inline std::pair<double, bool> exec_evaluate(const ScoreStatesTemp &states,
                                             const Restraints &restraints,
                                            const RestraintSets &restraint_sets,
                                             double omax,
                                             Model *m) {
    IMP_FUNCTION_LOG;
    IMP_CHECK_OBJECT(m);
    // make sure stage is restored on an exception
    internal::SFSetIt<IMP::internal::Stage, internal::NOT_EVALUATING>
      reset(&m->cur_stage_);
    IMP_CHECK_OBJECT(m);
    IMP_LOG(VERBOSE, "On restraints " << restraints
            << " restraint sets " << restraint_sets
            << " and score states " << states
            << std::endl);
    m->before_evaluate(get_as<ScoreStatesTemp>(states));

    m->cur_stage_= internal::EVALUATING;
    if (DERIV) {
      m->zero_derivatives();
    }
    std::pair<double, bool> ret
        = exec_evaluate< DERIV, GOOD, MAX, STATS>(restraints,
                                              restraint_sets,
                                              omax, 1.0,
                                              m);

    m->after_evaluate(get_as<ScoreStatesTemp>(states), DERIV);

    // validate derivatives
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      if (DERIV) {
        m->validate_computed_derivatives();
      }
    }
    IMP_LOG(TERSE, "Final score: "
            << ret << std::endl);
    ++m->eval_count_;
    m->first_call_=false;
    return ret;
  }

}


RestraintsScoringFunction::RestraintsScoringFunction(const RestraintsTemp &r,
                                                     double weight,
                                                     double max,
                                                     std::string name):
    ScoringFunction(r[0]->get_model(), name),
    weight_(weight), max_(max){
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
  IMP_SF_CALL_EVALUATE(NO_MAX,
                    false, false);
}
std::pair<double, bool>
RestraintsScoringFunction::do_evaluate_if_good(bool derivatives,
                                        const ScoreStatesTemp &ss) {
  IMP_SF_CALL_EVALUATE(max_,
                    true, false);
}

std::pair<double, bool>
RestraintsScoringFunction::do_evaluate_if_below(bool derivatives,
                                         double max,
                                         const ScoreStatesTemp &ss) {
  IMP_SF_CALL_EVALUATE(max,
                    false, true);
}


RestraintsTemp RestraintsScoringFunction::get_restraints() const {
  RestraintsTemp ret= get_as<RestraintsTemp>(rs_)
      + get_as<RestraintsTemp>(rss_);
  return ret;
}


void RestraintsScoringFunction::do_show(std::ostream &out) const {
  IMP_UNUSED(out);
}

ScoringFunctionInput::ScoringFunctionInput(Model *sf):
  P(IMP::internal::create_scoring_function(sf)){
}



ScoringFunctionInput::ScoringFunctionInput(const RestraintsTemp &sf):
    P(new RestraintsScoringFunction(sf)){
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

/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/kernel/Particle.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/log.h"
#include "IMP/kernel/Restraint.h"
#include "IMP/kernel/container_base.h"
#include "IMP/kernel/ScoringFunction.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/base//warning_macros.h"
#include <IMP/base//thread_macros.h>
#include "IMP/kernel/input_output.h"
#include "IMP/kernel/internal/RestraintsScoringFunction.h"
#include "IMP/base//Pointer.h"
#include <IMP/base//check_macros.h>
#include <numeric>

IMPKERNEL_BEGIN_NAMESPACE
const double NO_MAX=std::numeric_limits<double>::max();
const double BAD_SCORE=NO_MAX;


Restraint::Restraint(Model *m, std::string name):
    ModelObject(m, name), weight_(1), max_(NO_MAX),
  last_score_(BAD_SCORE)
{
}

Restraint::Restraint(std::string name):
  ModelObject(name), weight_(1), max_(NO_MAX),
  last_score_(BAD_SCORE)
{
  IMP_WARN("You should pass the model to the Restraint constructor. "
           << "Constructing " << name << std::endl);
}

// for model
Restraint::Restraint(ModelInitTag, std::string name):
  ModelObject(name), weight_(1), max_(NO_MAX),
  last_score_(BAD_SCORE)
{
}

double Restraint::evaluate(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  Pointer<ScoringFunction> sf = create_scoring_function();
  return sf->evaluate(calc_derivs);
}


double Restraint::evaluate_if_good(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  Pointer<ScoringFunction> sf = create_scoring_function();
  return sf->evaluate_if_good(calc_derivs);
}

double Restraint::evaluate_if_below(bool calc_derivs, double max) const {
  IMP_OBJECT_LOG;
  Pointer<ScoringFunction> sf = create_scoring_function();
  return sf->evaluate_if_below(calc_derivs, max);
}

double Restraint::unprotected_evaluate(DerivativeAccumulator *da) const{
  IMP_USAGE_CHECK(!da,
                  "Do not call unprotected evaluate directly if you"
                  << " want derivatives.");
  IMP_CHECK_VARIABLE(da);
  EvaluationState es(0,true);
  ScoreAccumulator sa(&es, 1, false, NO_MAX, NO_MAX, false);
  do_add_score_and_derivatives(sa);
  return es.score;
}

void Restraint::set_weight(double w) {
  if (w != weight_) {
    if (get_is_part_of_model()) {
      get_model()->clear_caches();
    }
    weight_=w;
  }
}

void Restraint::set_maximum_score(double w) {
  if (w != max_) {
    if (get_is_part_of_model()) {
      get_model()->clear_caches();
    }
    max_=w;
  }
}

namespace {
  void check_decomposition(Restraint *in, Restraint *out) {
    IMP_INTERNAL_CHECK(out->get_is_part_of_model(),
                       "Restraint " << out->get_name()
                       << " produced from " << in->get_name()
                       << " is not already part of model.");
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
      // be lazy and hope that they behave the same on un updated states
      // otherwise it can be bery, bery slow
      //in->get_model()->update();
      base::SetLogState sls(WARNING);
      double tin= in->unprotected_evaluate(nullptr);
      double tout= out->unprotected_evaluate(nullptr);
      if (std::abs(tin-tout) > .01*std::abs(tin+tout)+.1) {
        IMP_WARN("The before and after scores don't agree for: \""
                 << in->get_name() << "\" got "
                 << tin << " and " << tout << std::endl);
        IMP_LOG_WRITE(WARNING, show_restraint_hierarchy(in, IMP_STREAM));
        IMP_LOG_WRITE(WARNING, show_restraint_hierarchy(out, IMP_STREAM));
      }
    }
  }


  Restraint* create_decomp_helper(const Restraint* me,
                                  const Restraints &created) {
    if (created.empty()) return nullptr;
    if (created.size()== 1) {
      if (created[0]!= me) {
        /** We need to special case this here, otherwise repeatedly calling
            decompositions causes bad things to happen (recursion) */
        double weight=created[0]->get_weight()*me->get_weight();
        double max= std::min(created[0]->get_maximum_score(),
                             me->get_maximum_score()/created[0]->get_weight());
        created[0]->set_weight(weight);
        created[0]->set_maximum_score(max);
        created[0]->set_model(me->get_model());
        created[0]->set_log_level(me->get_log_level());
        created[0]->set_check_level(me->get_check_level());
      }
      check_decomposition(const_cast<Restraint*>(me), created[0]);
      return created[0];
    } else {
      IMP_NEW(RestraintSet, rs, (me->get_name() + " decomposition"));
      IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
        for (unsigned int i=0; i< created.size(); ++i) {
          IMP_INTERNAL_CHECK(created[i],
                             "nullptr restraint returned in decomposition");
        }
      }
      for (unsigned int i=0; i < created.size(); ++i) {
        created[i]->set_log_level(me->get_log_level());
        created[i]->set_check_level(me->get_check_level());
      }
      rs->set_log_level(me->get_log_level());
      rs->set_check_level(me->get_check_level());
      rs->add_restraints(created);
      rs->set_maximum_score(me->get_maximum_score());
      rs->set_weight(me->get_weight());
      rs->set_model(me->get_model());
      check_decomposition(const_cast<Restraint*>(me), rs);
      return rs.release();
    }
  }
}

Restraint* Restraint::create_decomposition() const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  base::Pointer<Restraint> ret= create_decomp_helper(this,
                                                     do_create_decomposition());
  return ret.release();
}


Restraint* Restraint::create_current_decomposition() const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  Restraints rs=do_create_current_decomposition();
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  for (unsigned int i=0; i< rs.size(); ++i) {
    double old_score= rs[i]->get_last_score();
    double new_score= rs[i]->unprotected_evaluate(nullptr);
    IMP_INTERNAL_CHECK(new_score != 0,
                       "The score of the current decomposition term is 0."
                       << " This is unacceptable.");
    IMP_INTERNAL_CHECK_FLOAT_EQUAL(old_score, new_score,
                                   "Old and new scores don't match");
  }
#endif
  // need pointer to make sure destruction of rs doesn't free anything
  base::Pointer<Restraint> ret= create_decomp_helper(this,
                                                     rs);
  rs.clear(); // must be done before release to avoid frees
  return ret.release();
}

ScoringFunction *Restraint::create_scoring_function(double weight,
                                                    double max ) const {
  if (weight == 1.0 && max == NO_MAX) {
    if (!cached_scoring_function_) {
      Restraint* ncthis= const_cast<Restraint*>(this);
      IMP_NEW(internal::GenericRestraintsScoringFunction<RestraintsTemp>, ret,
              (RestraintsTemp(1, ncthis),
               1.0, NO_MAX,
               get_name()+" scoring"));
      cached_scoring_function_= ret;
    }
    return cached_scoring_function_;
  } else {
    Restraint* ncthis= const_cast<Restraint*>(this);
    IMP_NEW(internal::RestraintsScoringFunction, ret,
            (RestraintsTemp(1, ncthis),
             weight, max,
             get_name()+" scoring"));
    return ret.release();
  }
}

Restraints create_decomposition(const RestraintsTemp &rs) {
  Restraints ret;
  for (unsigned int i=0; i< rs.size(); ++i) {
    base::Pointer<Restraint> r= rs[i]->create_decomposition();
    if (r) {
      ret.push_back(r);
    }
  }
  return ret;
}

void Restraint::do_add_score_and_derivatives(ScoreAccumulator sa) const {
  IMP_OBJECT_LOG;
  if (!sa.get_abort_evaluation()) {
    double score;
    if (sa.get_is_evaluate_if_below()) {
      score= unprotected_evaluate_if_below(sa.get_derivative_accumulator(),
                                           sa.get_maximum());
    } else if (sa.get_is_evaluate_if_good()) {
      score= unprotected_evaluate_if_good(sa.get_derivative_accumulator(),
                                          sa.get_maximum());
    } else {
      score= unprotected_evaluate(sa.get_derivative_accumulator());
    }
    IMP_LOG_TERSE( "Adding " << score << " from restraint " << get_name()
            << std::endl);
    sa.add_score(score);
    set_last_score(score);
  }
}

double Restraint::get_score() const {
  return evaluate(false);
}

void Restraint::add_score_and_derivatives(ScoreAccumulator sa) const {
  // implement these in macros to avoid extra virtual function call
  ScoreAccumulator nsa(sa, this);
  IMP_TASK((nsa), do_add_score_and_derivatives(nsa),
           "add score and derivatives");
  set_was_used(true);
}

#if IMP_HAS_DEPRECATED
ParticlesTemp Restraint::get_input_particles() const {
  IMP_DEPRECATED_FUNCTION(get_inputs());
  return IMP::kernel::get_input_particles(get_inputs());
}
ContainersTemp Restraint::get_input_containers() const {
  IMP_DEPRECATED_FUNCTION(get_inputs());
  return IMP::kernel::get_input_containers(get_inputs());
}
#endif


IMPKERNEL_END_NAMESPACE

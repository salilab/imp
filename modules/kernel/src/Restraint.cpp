/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/ScoringFunction.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/RestraintsScoringFunction.h"
#include "IMP/base/Pointer.h"
#include <IMP/base/check_macros.h>
#include <numeric>

IMP_BEGIN_NAMESPACE
const double NO_MAX=std::numeric_limits<double>::max();
const double BAD_SCORE=NO_MAX;


Restraint::Restraint(Model *m, std::string name):
    Tracked(this, m, name), weight_(1), max_(NO_MAX),
  last_score_(BAD_SCORE)
{
}


Restraint::Restraint(std::string name):
  Tracked(name), weight_(1), max_(NO_MAX),
  last_score_(BAD_SCORE)
{
}

void Restraint::set_model(Model* model)
{
  Tracked::set_tracker(this, model);
}

namespace {
  double finish(const Floats &ret,
              const Restraint *me) {
    double rv=std::accumulate(ret.begin(), ret.begin()+ret.size(), 0.0);
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "Score is " << rv << "\n");
      for (unsigned int i=0; i< ret.size(); ++i) {
        IMP_LOG(VERBOSE, ret[i] << " for " << me->get_name() << "\n");
      }
    }
    return rv;
  }
}

double Restraint::evaluate(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  base::Pointer<ScoringFunction> sf= create_scoring_function();
  return sf->evaluate(calc_derivs);
}


double Restraint::evaluate_if_good(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  base::Pointer<ScoringFunction> sf= create_scoring_function();
  return sf->evaluate_if_good(calc_derivs);
}

double Restraint::evaluate_if_below(bool calc_derivs, double max) const {
  IMP_OBJECT_LOG;
  base::Pointer<ScoringFunction> sf= create_scoring_function(1.0, max);
  return sf->evaluate_if_below(calc_derivs, max);
}

void Restraint::set_weight(double w) {
  weight_=w;
  if (get_is_part_of_model()) {
    get_model()->reset_dependencies();
  }
}

void Restraint::set_maximum_score(double w) {
  max_=w;
  if (get_is_part_of_model()) {
    get_model()->reset_dependencies();
  }
}

namespace {
  void check_decomposition(Restraint *in, Restraint *out) {
    IMP_INTERNAL_CHECK(out->get_is_part_of_model(),
                       "Restraint " << out->get_name()
                       << " produced from " << in->get_name()
                       << " is not already part of model.");
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
      in->get_model()->update();
      /*IMP_PRINT_TREE(std::cout, Restraint*, in,
                     (dynamic_cast<RestraintSet*>(n)?
                      dynamic_cast<RestraintSet*>(n)->get_number_of_restraints()
                      :0),
                     dynamic_cast<RestraintSet*>(n)->get_restraint,
                     std::cout << n->get_name() << ": " << n->get_weight());
      IMP_PRINT_TREE(std::cout, Restraint*, out,
                     (dynamic_cast<RestraintSet*>(n)?
                      dynamic_cast<RestraintSet*>(n)->get_number_of_restraints()
                      :0),
                     dynamic_cast<RestraintSet*>(n)->get_restraint,
                     std::cout << n->get_name() << ": " << n->get_weight());*/
      base::SetLogState sls(WARNING);
      double tin= in->unprotected_evaluate(nullptr);
      double tout= out->unprotected_evaluate(nullptr);
      if (std::abs(tin-tout) > .01*std::abs(tin+tout)+.1) {
        IMP_WARN("The before and after scores don't agree for: \""
                 << in->get_name() << "\" got "
                 << tin << " and " << tout << " over " << std::endl);
      }
    }
  }


  Restraint* create_decomp_helper(const Restraint* me,
                                  const Restraints &created) {
    if (created.empty()) return NULL;
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
      }
      check_decomposition(const_cast<Restraint*>(me), created[0]);
      return created[0];
    } else {
      IMP_NEW(RestraintSet, rs, (me->get_name() + " decomposition"));
      IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
        for (unsigned int i=0; i< created.size(); ++i) {
          IMP_INTERNAL_CHECK(created[i],
                             "NULL restraint returned in decomposition");
        }
      }
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
  base::Pointer<Restraint> ret= create_decomp_helper(this,
                                                     do_create_decomposition());
  return ret.release();
}


Restraint* Restraint::create_current_decomposition() const {
  base::Pointer<Restraint> ret= create_decomp_helper(this,
                                     do_create_current_decomposition());
  return ret.release();
}

ScoringFunction *Restraint::create_scoring_function(double weight,
                                                    double max ) const {
  Restraint* ncthis= const_cast<Restraint*>(this);
  return new internal::RestraintsScoringFunction(RestraintsTemp(1, ncthis),
                                       weight, max);
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

IMP_END_NAMESPACE

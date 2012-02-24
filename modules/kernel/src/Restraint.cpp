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
#include "IMP/internal/utility.h"
#include <boost/tuple/tuple.hpp>
#include <numeric>

IMP_BEGIN_NAMESPACE

Restraint::Restraint(std::string name):
  Object(name), weight_(1), max_(std::numeric_limits<double>::max()),
  model_weight_(-1)
{
}

Restraint::~Restraint()
{
  if (model_) {
    IMP_LOG(VERBOSE, "Removing tracked restraint " << get_name()
            << " from model." << std::endl);
    IMP_CHECK_OBJECT(model_);
    model_->remove_tracked_restraint(this);
  }
}


void Restraint::set_model(Model* model)
{
  IMP_USAGE_CHECK(!model || !model_
                  || (model_ && model_ == model),
                  "Model* different from already stored model "
                  << model->get_name() << " " << model_->get_name());
  if (model==model_) return;
  if (model_) {
    model_->remove_tracked_restraint(this);
  }
  model_=model;
  if (model_) {
    set_was_used(true);
    model_->add_tracked_restraint(this);
  }
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
  IMP_USAGE_CHECK(get_model(), "You must add the restraint to the"
                  << " model before attempting to evaluate it."
                  << " Use either Model::add_restraint() or "
                  << "Model::add_temporary_restraint().");
  Floats ret= get_model()
      ->evaluate(RestraintsTemp(1, const_cast<Restraint*>(this)),
                 calc_derivs);
  return finish(ret, this);
}


double Restraint::evaluate_if_good(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  Floats ret
      = get_model()
      ->evaluate_if_good(RestraintsTemp(1, const_cast<Restraint*>(this)),
                         calc_derivs);
  return finish(ret, this);
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
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
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
      Floats sin= in->get_model()->evaluate(RestraintsTemp(1, in), false);
      Floats sout= in->get_model()->evaluate(RestraintsTemp(1, out), false);
      // correct for it having a weight in the model
      double tin= std::accumulate(sin.begin(), sin.end(), 0.0);
      double tout= std::accumulate(sout.begin(), sout.end(), 0.0);
      if (std::abs(tin-tout) > .01*std::abs(tin+tout)+.1) {
        IMP_WARN("The before and after scores don't agree for: \""
                 << in->get_name() << "\" got "
                 << tin << " and " << tout << " over "
                 << sin << " vs " << sout << std::endl);
      }
    }
  }
}

namespace {
  Restraint* create_decomp_helper(const Restraint* me,
                                  const Restraints &created) {
    if (created.empty()) return NULL;
    if (created.size()== 1 && created[0]==me) {
      return created[0];
    } else {
      IMP_NEW(RestraintSet, rs, (me->get_name() + " decomposed"));
      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        for (unsigned int i=0; i< created.size(); ++i) {
          IMP_INTERNAL_CHECK(created[i],
                             "NULL restraint returned in decomposition");
        }
      }
      for (unsigned int i=0; i< created.size(); ++i) {
        created[i]->set_maximum_score(me->get_maximum_score());
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
  Pointer<Restraint> ret= create_decomp_helper(this, do_create_decomposition());
  return ret.release();
}


Restraint* Restraint::create_incremental_decomposition(unsigned int n) const {
  Pointer<Restraint> ret
    = create_decomp_helper(this, do_create_incremental_decomposition(n));
  return ret.release();
}


Restraint* Restraint::create_current_decomposition() const {
  Pointer<Restraint> ret= create_decomp_helper(this,
                                     do_create_current_decomposition());
  return ret.release();
}



IMP_END_NAMESPACE

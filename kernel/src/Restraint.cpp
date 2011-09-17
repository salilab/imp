/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
  model_weight_(0)
{
}

Restraint::~Restraint()
{
  IMP_OBJECT_LOG;
  if (model_) {
    IMP_LOG(TERSE, "Removing trackedf restraint " << get_name()
            << " from model." << std::endl);
    IMP_CHECK_OBJECT(model_);
    model_->remove_tracked_restraint(this);
  } else {
     IMP_LOG(TERSE, "Not removing restraint " << get_name()
            << " from model." << std::endl);
  }
}


void Restraint::set_model(Model* model)
{
  IMP_USAGE_CHECK(!model || !model_
                  || (model_ && model_ == model),
                  "Model* different from already stored model "
                  << model << " " << model_);
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
  void fill_restraints(RestraintsTemp &restraints,
                       const Restraint *me) {
    restraints= get_restraints(RestraintsTemp(1, const_cast<Restraint*>(me)));
    if (restraints.size() >1) {
      IMP_LOG(VERBOSE, "Evaluating "<< restraints.size()
              << " restraints in set.\n");
    }
  }
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
  RestraintsTemp restraints;
  fill_restraints(restraints, this);
  IMP_USAGE_CHECK(get_model(), "You must add the restraint to the"
                  << " model before attempting to evaluate it."
                  << " Use either Model::add_restraint() or "
                  << "Model::add_temporary_restraint().");
  Floats ret= get_model()->evaluate(restraints, calc_derivs);
  return finish(ret, this);
}


double Restraint::evaluate_if_good(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  RestraintsTemp restraints;
  fill_restraints(restraints, this);
  Floats ret= get_model()->evaluate_if_good(restraints, calc_derivs);
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
      RestraintsTemp frs= get_restraints(RestraintsTemp(1, in));
      RestraintsTemp fret= get_restraints(RestraintsTemp(1, out));
      IMP_LOG(TERSE, "Evaluating before" << std::endl);
      Floats efrs= in->get_model()->evaluate(frs, false);
      IMP_LOG(TERSE, "Evaluating after" << std::endl);
      Floats efret= in->get_model()->evaluate(fret, false);
      double s0= std::accumulate(efrs.begin(), efrs.end(), 0.0);
      double s1= std::accumulate(efret.begin(), efret.end(), 0.0);
      if (std::abs(s0-s1) > .1*std::abs(s0+s1)+.1) {
        IMP_WARN("The before and after scores don't agree for: "
                 << in->get_name() << " got "
                 << s0 << " and " << s1 << " over "
                 << efrs << " vs " << efret);
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

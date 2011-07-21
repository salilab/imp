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


void Restraint::set_model(Model* model)
{
  IMP_USAGE_CHECK(model==NULL || model_==NULL
            || (model_ && model_ == model),
            "Model* different from already stored model "
            << model << " " << model_);
  model_=model;
  if (model) set_was_used(true);
}

namespace {
  void fill_restraints(RestraintsTemp &restraints,
                       const Restraint *me) {
    if (dynamic_cast<const RestraintSet*>(me)) {
      restraints=
        get_restraints(dynamic_cast<const RestraintSet*>(me));
    } else {
      restraints.push_back(const_cast<Restraint*>(me));
    }
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



IMP_END_NAMESPACE

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
  Object(name), weight_(1), max_(std::numeric_limits<double>::max())
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
  void fill_restraints_and_weights(RestraintsTemp &restraints,
                                   std::vector<double> &weights,
                                   const Restraint *me) {
    if (dynamic_cast<const RestraintSet*>(me)) {
      boost::tie(restraints, weights)=
        get_restraints_and_weights(dynamic_cast<const RestraintSet*>(me));
    } else {
      restraints.push_back(const_cast<Restraint*>(me));
      weights.push_back(restraints.back()->get_weight());
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
  std::vector<double> weights;
  fill_restraints_and_weights(restraints, weights, this);
  Floats ret= get_model()->evaluate(restraints, weights, calc_derivs);
  return finish(ret, this);
}


double Restraint::evaluate_if_good(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  RestraintsTemp restraints;
  std::vector<double> weights;
  fill_restraints_and_weights(restraints, weights, this);
  Floats ret= get_model()->evaluate_if_good(restraints, weights, calc_derivs);
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

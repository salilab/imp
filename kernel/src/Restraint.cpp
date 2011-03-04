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

double Restraint::evaluate(bool calc_derivs) const {
  IMP_OBJECT_LOG;
  RestraintsTemp restraints;
  std::vector<double> weights;
  if (dynamic_cast<const RestraintSet*>(this)) {
    boost::tie(restraints, weights)=
      get_restraints_and_weights(dynamic_cast<const RestraintSet*>(this));
  } else {
    restraints.push_back(const_cast<Restraint*>(this));
    weights.push_back(get_weight());
  }
  IMP_LOG(VERBOSE, "Evaluating "<< restraints.size()
          << " restraints in set.\n");
  Floats ret= get_model()->evaluate(restraints, weights, calc_derivs);
  double rv=std::accumulate(ret.begin(), ret.begin()+ret.size(), 0.0);
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Got " << rv << "\n");
    for (unsigned int i=0; i< ret.size(); ++i) {
      IMP_LOG(VERBOSE, ret[i] << " for " << restraints[i]->get_name() << "\n");
    }
  }
  return rv;
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

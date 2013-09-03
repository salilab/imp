/**
 *  \file MaintainScaleOrderConstraint.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/isd/MaintainScaleOrderConstraint.h"
#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

MaintainScaleOrderConstraint::MaintainScaleOrderConstraint(
                                         const kernel::Particles& p,
                                         std::string name):
  Constraint(name), p_(p){}


void MaintainScaleOrderConstraint::do_update_attributes()
{
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE( "Begin MaintainScaleOrderConstraint::update" << std::endl);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned i=0; i<p_.size(); i++){
        Scale::decorate_particle(p_[i]);
      }
    }
    //get scale values
    Scales s;
    std::vector<double> val;
    for (unsigned i=0; i<p_.size(); i++) {
        s.push_back(Scale(p_[i]));
        val.push_back(s[i].get_scale());
    }
    //sort the scale values and set them back. set_scale takes care that they
    //are all bounded correctly.
    //Does not guarantee sorting if scales have different bounds.
    std::sort(val.begin(), val.end());
    for (unsigned i=0; i<p_.size(); i++){
        s[i].set_scale(val[i]);
    }
    IMP_LOG_TERSE( "End MaintainScaleOrderConstraint::update" << std::endl);
}

void MaintainScaleOrderConstraint::do_update_derivatives(
                                               DerivativeAccumulator *){}

ModelObjectsTemp MaintainScaleOrderConstraint::do_get_outputs() const {
  return kernel::ModelObjectsTemp(p_.begin(), p_.end());
}

ModelObjectsTemp MaintainScaleOrderConstraint::do_get_inputs() const {
  return kernel::ModelObjectsTemp(p_.begin(), p_.end());
}
IMPISD_END_NAMESPACE

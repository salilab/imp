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
                                         const Particles& p,
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

ContainersTemp MaintainScaleOrderConstraint::get_input_containers() const {
  return ContainersTemp();
}

ContainersTemp MaintainScaleOrderConstraint::get_output_containers() const {
  return ContainersTemp();
}


ParticlesTemp MaintainScaleOrderConstraint::get_input_particles() const {
  ParticlesTemp ret(p_.begin(), p_.end());
  return ret;
}

ParticlesTemp MaintainScaleOrderConstraint::get_output_particles() const {
  ParticlesTemp ret(p_.begin(), p_.end());
  return ret;
}

void MaintainScaleOrderConstraint::do_show(std::ostream &out) const {
  out << "MaintainScaleOrderConstraint on " ;
  for (unsigned i=0; i<p_.size(); i++){
      out << p_[i] << " ";
  }
  out << std::endl;
}

IMPISD_END_NAMESPACE

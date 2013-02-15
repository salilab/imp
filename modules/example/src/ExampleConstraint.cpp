/**
 *  \file example/ExampleConstraint.cpp
 *  \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/example/ExampleConstraint.h>
#include <IMP/core/XYZ.h>

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleConstraint::ExampleConstraint(Particle *p) :
  p_(p), k_("Constraint key") {
  if (!p_->has_attribute(k_)) {
    p_->add_attribute(k_, 0);
  }
}


void ExampleConstraint::do_update_attributes() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Updating example constraint with particle "
          << p_->get_value(k_) << std::endl);
  p_->set_value(k_, p_->get_value(k_)+1);
}
void ExampleConstraint::do_update_derivatives(DerivativeAccumulator *) {

}
ContainersTemp ExampleConstraint::get_input_containers() const {
  return ContainersTemp();
}
ContainersTemp ExampleConstraint::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp ExampleConstraint::get_input_particles() const {
  return ParticlesTemp(1, p_);
}
ParticlesTemp ExampleConstraint::get_output_particles() const {
  return ParticlesTemp(1, p_);
}
void ExampleConstraint::do_show(std::ostream &out) const {
  out << "current count= " << p_->get_value(k_) << std::endl;
}


IMPEXAMPLE_END_NAMESPACE

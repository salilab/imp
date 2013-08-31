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

ExampleConstraint::ExampleConstraint(kernel::Particle *p)
    : p_(p), k_("Constraint key") {
  if (!p_->has_attribute(k_)) {
    p_->add_attribute(k_, 0);
  }
}

void ExampleConstraint::do_update_attributes() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Updating example constraint with particle "
                << p_->get_value(k_) << std::endl);
  p_->set_value(k_, p_->get_value(k_) + 1);
}
void ExampleConstraint::do_update_derivatives(DerivativeAccumulator *) {}
ModelObjectsTemp ExampleConstraint::do_get_inputs() const {
  return ModelObjectsTemp(1, p_);
}
ModelObjectsTemp ExampleConstraint::do_get_outputs() const {
  return ModelObjectsTemp(1, p_);
}

IMPEXAMPLE_END_NAMESPACE

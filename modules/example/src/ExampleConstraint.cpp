/**
 *  \file example/ExampleConstraint.cpp
 *  \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/example/ExampleConstraint.h>
#include <IMP/core/XYZ.h>

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleConstraint::ExampleConstraint(Particle *p)
    : Constraint(p->get_model(), "ExampleConstraint%1%"),
      p_(p),
      k_(get_key()) {
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

IntKey ExampleConstraint::get_key() {
  static IntKey k("Constraint key");
  return k;
}

IMPEXAMPLE_END_NAMESPACE

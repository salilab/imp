/**
 *  \file example/ExampleConstraint.cpp
 *  \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/example/ExampleConstraint.h>
#include <IMP/core/XYZ.h>

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleConstraint::ExampleConstraint(Particle *p)
    : Constraint(p->get_model(), "ExampleConstraint%1%"),
      p_(p->get_index()),
      k_(get_key()) {
  if (!p->has_attribute(k_)) {
    p->add_attribute(k_, 0);
  }
}

void ExampleConstraint::do_update_attributes() {
  IMP_OBJECT_LOG;
  Particle *p = get_model()->get_particle(p_);
  IMP_LOG_TERSE("Updating example constraint with particle "
                << p->get_value(k_) << std::endl);
  p->set_value(k_, p->get_value(k_) + 1);
}

void ExampleConstraint::do_update_derivatives(DerivativeAccumulator *) {}

ModelObjectsTemp ExampleConstraint::do_get_inputs() const {
  Particle *p = get_model()->get_particle(p_);
  return ModelObjectsTemp(1, p);
}
ModelObjectsTemp ExampleConstraint::do_get_outputs() const {
  Particle *p = get_model()->get_particle(p_);
  return ModelObjectsTemp(1, p);
}

IntKey ExampleConstraint::get_key() {
  static IntKey k("Constraint key");
  return k;
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::example::ExampleConstraint);

IMPEXAMPLE_END_NAMESPACE

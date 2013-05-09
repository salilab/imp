/**
 *  \file ConstantRestraint.cpp \brief Don't restrain anything.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/ConstantRestraint.h>

IMPCORE_BEGIN_NAMESPACE

ConstantRestraint::ConstantRestraint(Float v) : v_(v) {}

double ConstantRestraint::unprotected_evaluate(DerivativeAccumulator*) const {
  return v_;
}

ParticlesTemp ConstantRestraint::get_input_particles() const {
  return ParticlesTemp();
}

ContainersTemp ConstantRestraint::get_input_containers() const {
  return ContainersTemp();
}

void ConstantRestraint::do_show(std::ostream& out) const {
  out << "value is " << v_ << std::endl;
}

IMPCORE_END_NAMESPACE

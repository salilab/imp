/**
 *  \file ConstantRestraint.cpp \brief Don't restrain anything.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/ConstantRestraint.h>

IMPCORE_BEGIN_NAMESPACE

ConstantRestraint::ConstantRestraint(Model *m, Float v)
    : Restraint(m, "ConstantRestraint%1%"), v_(v) {}

double ConstantRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  return v_;
}

ModelObjectsTemp ConstantRestraint::do_get_inputs() const {
  return kernel::ModelObjectsTemp();
}

IMPCORE_END_NAMESPACE

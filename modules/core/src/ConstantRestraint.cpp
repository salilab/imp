/**
 *  \file ConstantRestraint.cpp \brief Don't restrain anything.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/ConstantRestraint.h>

IMPCORE_BEGIN_NAMESPACE

ConstantRestraint::ConstantRestraint(Float v) : v_(v){}



Float ConstantRestraint::evaluate(DerivativeAccumulator *accum)
{
  return v_;
}

void ConstantRestraint::show(std::ostream& out) const
{
  out << "Constant restraint :" << v_ << std::endl;
}

IMPCORE_END_NAMESPACE

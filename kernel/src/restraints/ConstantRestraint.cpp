/**
 *  \file ConstantRestraint.cpp \brief Don't restrain anything.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/ConstantRestraint.h"

IMP_BEGIN_NAMESPACE

ConstantRestraint::ConstantRestraint(Float v) : v_(v){}



Float ConstantRestraint::evaluate(DerivativeAccumulator *accum)
{
  return v_;
}

void ConstantRestraint::show(std::ostream& out) const
{
  out << "Constant restraint :" << v_ << std::endl;
}

IMP_END_NAMESPACE

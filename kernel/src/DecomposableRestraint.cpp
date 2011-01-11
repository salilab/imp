/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include "IMP/DecomposableRestraint.h"

IMP_BEGIN_NAMESPACE

DecomposableRestraint::DecomposableRestraint(std::string name):
  Restraint(name)
{
}

double DecomposableRestraint
::unprotected_evaluate_subset(DerivativeAccumulator *da) const {
  IMP_NOT_IMPLEMENTED;
}

IMP_END_NAMESPACE

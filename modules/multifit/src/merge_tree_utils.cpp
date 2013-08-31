/**
 *  \file merge_tree_utils.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/multifit/merge_tree_utils.h>

IMPMULTIFIT_BEGIN_NAMESPACE

double DummyRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{return 0.;
}
ModelObjectsTemp DummyRestraint::do_get_inputs() const
{
  kernel::ParticlesTemp pt;
  pt.push_back(p1_);
  pt.push_back(p2_);
  return pt;
}

IMPMULTIFIT_END_NAMESPACE

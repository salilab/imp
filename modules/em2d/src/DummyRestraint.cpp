/**
 *  \file DummyRestraint.cpp
 *  \brief Dummy restraint
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/DummyRestraint.h"


IMPEM2D_BEGIN_NAMESPACE

double
DummyRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  return 0;
}

ModelObjectsTemp DummyRestraint::do_get_inputs() const
{
  ParticlesTemp ret;
  ret.push_back(p0_);
  ret.push_back(p1_);
  return ret;
}

double
ParticlesDummyRestraint::unprotected_evaluate(
                                      DerivativeAccumulator *) const {
  return 0;
}

ModelObjectsTemp ParticlesDummyRestraint::do_get_inputs() const
{
  ModelObjectsTemp ret= container_->get_particles();
  ret.push_back(container_);
  return ret;
}

IMPEM2D_END_NAMESPACE

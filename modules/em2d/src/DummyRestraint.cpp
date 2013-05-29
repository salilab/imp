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

void DummyRestraint::do_show(std::ostream& out) const
{
  out << "Dummy" << std::endl;
}


ParticlesTemp DummyRestraint::get_input_particles() const
{
  ParticlesTemp ret;
  ret.push_back(p0_);
  ret.push_back(p1_);
  return ret;
}

ContainersTemp DummyRestraint::get_input_containers() const
{
  // Returns a vector of one container with the particles
  return ContainersTemp();
}

double
ParticlesDummyRestraint::unprotected_evaluate(
                                      DerivativeAccumulator *) const {
  return 0;
}

void ParticlesDummyRestraint::do_show(std::ostream& out) const
{
  out << "ParticlesDummyRestraint" << std::endl;
}


ParticlesTemp ParticlesDummyRestraint::get_input_particles() const
{
  ParticlesTemp ret= container_->get_particles();
  return ret;
}

ContainersTemp ParticlesDummyRestraint::get_input_containers() const
{
  // Returns a vector of one container with the particles
  return ContainersTemp(1, container_);
}


IMPEM2D_END_NAMESPACE

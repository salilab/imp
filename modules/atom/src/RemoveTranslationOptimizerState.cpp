/**
 *  \file RemoveTranslationOptimizerState.cpp
 *  \brief Remove rigid translation during optimization.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/RemoveTranslationOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

RemoveTranslationOptimizerState::RemoveTranslationOptimizerState(
    const Particles &pis, unsigned skip_steps)
    : pis_(pis), skip_steps_(skip_steps), call_number_(0) {}

void RemoveTranslationOptimizerState::update() {
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    remove_translation();
  }
  ++call_number_;
}

void RemoveTranslationOptimizerState::remove_translation() const {
  Particle *p0 = *pis_.begin();
  core::XYZ d0(p0);
  algebra::Vector3D coords = d0.get_coordinates();

  for (Particles::const_iterator pi = pis_.begin(); pi != pis_.end(); ++pi) {
    Particle *p = *pi;
    core::XYZ d(p);
    d.set_coordinates(d.get_coordinates() - coords);
  }
}

IMPATOM_END_NAMESPACE

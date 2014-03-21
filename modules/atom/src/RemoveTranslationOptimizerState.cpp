/**
 *  \file RemoveTranslationOptimizerState.cpp
 *  \brief Remove rigid translation during optimization.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/RemoveTranslationOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

RemoveTranslationOptimizerState::RemoveTranslationOptimizerState(
    kernel::Model *m, kernel::ParticleIndexesAdaptor pis)
    : kernel::OptimizerState(m, "RemoveTranslationOptimizerState%1%") {
  IMP_FOREACH(kernel::ParticleIndex pi, pis) {
    pis_.push_back(m->get_particle(pi));
  }
}

void RemoveTranslationOptimizerState::do_update(unsigned int) {
  remove_translation();
}

void RemoveTranslationOptimizerState::remove_translation() const {
  set_was_used(true);
  kernel::Particle *p0 = *pis_.begin();
  core::XYZ d0(p0);
  algebra::Vector3D coords = d0.get_coordinates();

  for (kernel::Particles::const_iterator pi = pis_.begin(); pi != pis_.end();
       ++pi) {
    kernel::Particle *p = *pi;
    core::XYZ d(p);
    d.set_coordinates(d.get_coordinates() - coords);
  }
}

IMPATOM_END_NAMESPACE

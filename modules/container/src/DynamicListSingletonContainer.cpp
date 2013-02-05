/**
 *  \file ListSingletonContainer.cpp   \brief A list of ParticlesTemp.
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListSingletonContainer.h"
#include "IMP/SingletonModifier.h"
#include "IMP/SingletonScore.h"
#include <IMP/internal/InternalListSingletonContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListSingletonContainer
::DynamicListSingletonContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListSingletonContainer
::add_particle(Particle* vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListSingletonContainer
::add_particles(const ParticlesTemp &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListSingletonContainer
::set_particles(ParticlesTemp c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListSingletonContainer
::clear_particles() {
  clear();
}
IMPCONTAINER_END_NAMESPACE

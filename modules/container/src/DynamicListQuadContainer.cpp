/**
 *  \file ListQuadContainer.cpp   \brief A list of ParticleQuadsTemp.
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListQuadContainer.h"
#include "IMP/QuadModifier.h"
#include "IMP/QuadScore.h"
#include <IMP/internal/InternalListQuadContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListQuadContainer
::DynamicListQuadContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListQuadContainer
::add_particle_quad(const ParticleQuad& vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListQuadContainer
::add_particle_quads(const ParticleQuadsTemp &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListQuadContainer
::set_particle_quads(ParticleQuadsTemp c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListQuadContainer
::clear_particle_quads() {
  clear();
}
IMPCONTAINER_END_NAMESPACE

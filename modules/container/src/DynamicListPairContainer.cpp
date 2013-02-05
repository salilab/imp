/**
 *  \file ListPairContainer.cpp   \brief A list of ParticlePairsTemp.
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListPairContainer.h"
#include "IMP/PairModifier.h"
#include "IMP/PairScore.h"
#include <IMP/internal/InternalListPairContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListPairContainer
::DynamicListPairContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListPairContainer
::add_particle_pair(const ParticlePair& vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListPairContainer
::add_particle_pairs(const ParticlePairsTemp &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListPairContainer
::set_particle_pairs(ParticlePairsTemp c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListPairContainer
::clear_particle_pairs() {
  clear();
}
IMPCONTAINER_END_NAMESPACE

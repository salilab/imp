/**
 *  \file ListTripletContainer.cpp   \brief A list of ParticleTripletsTemp.
 *
 *  WARNING This file was generated from DynamicListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListTripletContainer.h"
#include "IMP/TripletModifier.h"
#include "IMP/TripletScore.h"
#include <IMP/internal/InternalListTripletContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListTripletContainer
::DynamicListTripletContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListTripletContainer
::add_particle_triplet(const ParticleTriplet& vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListTripletContainer
::add_particle_triplets(const ParticleTripletsTemp &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListTripletContainer
::set_particle_triplets(ParticleTripletsTemp c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListTripletContainer
::clear_particle_triplets() {
  clear();
}
IMPCONTAINER_END_NAMESPACE

/**
 *  \file TripletContainer.cpp   \brief Container for triplet.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/TripletContainer.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/InternalListTripletContainer.h"
#include "IMP/TripletModifier.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/triplet_macros.h"

IMP_BEGIN_NAMESPACE


TripletContainer::TripletContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
TripletContainer::~TripletContainer(){
}

#ifndef IMP_NO_DEPRECATED
bool TripletContainer
::get_contains_particle_triplet(ParticleTriplet v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  ParticleIndexTriplet iv= IMP::internal::get_index(v);
  IMP_FOREACH_TRIPLET_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

ParticleTripletsTemp TripletContainer
::get_particle_triplets() const {
  IMP_DEPRECATED_FUNCTION(get_indexes());
  return IMP::internal::get_particle(get_model(),
                                     get_indexes());
}

unsigned int TripletContainer
::get_number_of_particle_triplets() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}
ParticleTriplet TripletContainer
::get_particle_triplet(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

TripletContainerAdaptor
::TripletContainerAdaptor(TripletContainer *c): P(c){}
TripletContainerAdaptor
::TripletContainerAdaptor(const ParticleTripletsTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListTripletContainer, c,
          (m, name));
  c->set(IMP::internal::get_index(t));
  P::operator=(c);
}

IMP_END_NAMESPACE

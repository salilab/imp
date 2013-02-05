/**
 *  \file TripletContainer.cpp   \brief Container for triplet.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/TripletContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/internal/InternalListTripletContainer.h"
#include "IMP/kernel/TripletModifier.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/triplet_macros.h"

IMPKERNEL_BEGIN_NAMESPACE


TripletContainer::TripletContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
TripletContainer::~TripletContainer(){
}

#if IMP_USE_DEPRECATED
bool TripletContainer
::get_contains_particle_triplet(ParticleTriplet v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  ParticleIndexTriplet iv= IMP::kernel::internal::get_index(v);
  IMP_FOREACH_TRIPLET_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

ParticleTripletsTemp TripletContainer
::get_particle_triplets() const {
  return IMP::kernel::internal::get_particle(get_model(),
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

bool TripletContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void TripletContainer
::apply_generic(const TripletModifier *m) const {
  apply(m);
}

void TripletContainer
::apply(const TripletModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


TripletContainerAdaptor
::TripletContainerAdaptor(TripletContainer *c): P(c){}
TripletContainerAdaptor
::TripletContainerAdaptor(const ParticleTripletsTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListTripletContainer, c,
          (m, name));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}

IMPKERNEL_END_NAMESPACE

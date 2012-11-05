/**
 *  \file SingletonContainer.cpp   \brief Container for singleton.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/SingletonContainer.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/InternalListSingletonContainer.h"
#include "IMP/SingletonModifier.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/singleton_macros.h"

IMP_BEGIN_NAMESPACE


SingletonContainer::SingletonContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
SingletonContainer::~SingletonContainer(){
}

#ifndef IMP_NO_DEPRECATED
bool SingletonContainer
::get_contains_particle(Particle* v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  ParticleIndex iv= IMP::internal::get_index(v);
  IMP_FOREACH_SINGLETON_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

ParticlesTemp SingletonContainer
::get_particles() const {
  return IMP::internal::get_particle(get_model(),
                                     get_indexes());
}

unsigned int SingletonContainer
::get_number_of_particles() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}
Particle* SingletonContainer
::get_particle(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

SingletonContainerAdaptor
::SingletonContainerAdaptor(SingletonContainer *c): P(c){}
SingletonContainerAdaptor
::SingletonContainerAdaptor(const ParticlesTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListSingletonContainer, c,
          (m, name));
  c->set(IMP::internal::get_index(t));
  P::operator=(c);
}

IMP_END_NAMESPACE

/**
 *  \file SingletonContainer.cpp   \brief Container for singleton.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/SingletonContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/internal/InternalListSingletonContainer.h"
#include "IMP/kernel/SingletonModifier.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/singleton_macros.h"

IMPKERNEL_BEGIN_NAMESPACE


SingletonContainer::SingletonContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
SingletonContainer::~SingletonContainer(){
}

ParticlesTemp SingletonContainer
::get_particles() const {
  return IMP::kernel::internal::get_particle(get_model(),
                                     get_indexes());
}

#if IMP_USE_DEPRECATED
bool SingletonContainer
::get_contains_particle(Particle* v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  ParticleIndex iv= IMP::kernel::internal::get_index(v);
  IMP_FOREACH_SINGLETON_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
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

bool SingletonContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void SingletonContainer
::apply_generic(const SingletonModifier *m) const {
  apply(m);
}

void SingletonContainer
::apply(const SingletonModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


SingletonContainerAdaptor
::SingletonContainerAdaptor(SingletonContainer *c): P(c){}
SingletonContainerAdaptor
::SingletonContainerAdaptor(const ParticlesTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListSingletonContainer, c,
          (m, name));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}

IMPKERNEL_END_NAMESPACE

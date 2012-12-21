/**
 *  \file QuadContainer.cpp   \brief Container for quad.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/QuadContainer.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/InternalListQuadContainer.h"
#include "IMP/QuadModifier.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/quad_macros.h"

IMP_BEGIN_NAMESPACE


QuadContainer::QuadContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
QuadContainer::~QuadContainer(){
}

#if IMP_USE_DEPRECATED
bool QuadContainer
::get_contains_particle_quad(ParticleQuad v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  ParticleIndexQuad iv= IMP::internal::get_index(v);
  IMP_FOREACH_QUAD_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

ParticleQuadsTemp QuadContainer
::get_particle_quads() const {
  return IMP::internal::get_particle(get_model(),
                                     get_indexes());
}

unsigned int QuadContainer
::get_number_of_particle_quads() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}

ParticleQuad QuadContainer
::get_particle_quad(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

bool QuadContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void QuadContainer
::apply_generic(const QuadModifier *m) const {
  apply(m);
}

void QuadContainer
::apply(const QuadModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


QuadContainerAdaptor
::QuadContainerAdaptor(QuadContainer *c): P(c){}
QuadContainerAdaptor
::QuadContainerAdaptor(const ParticleQuadsTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListQuadContainer, c,
          (m, name));
  c->set(IMP::internal::get_index(t));
  P::operator=(c);
}

IMP_END_NAMESPACE

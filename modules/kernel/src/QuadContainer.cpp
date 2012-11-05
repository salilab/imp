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

bool QuadContainer
::get_contains_particle_quad(ParticleQuad v) const {
  ParticleIndexQuad iv= IMP::internal::get_index(v);
  IMP_FOREACH_QUAD_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
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

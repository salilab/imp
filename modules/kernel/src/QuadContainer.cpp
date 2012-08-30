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

IMP_BEGIN_NAMESPACE


QuadContainer::QuadContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
QuadContainer::~QuadContainer(){
}

QuadContainerAdaptor
::QuadContainerAdaptor(QuadContainer *c): P(c){}
QuadContainerAdaptor
::QuadContainerAdaptor(const ParticleQuadsTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListQuadContainer, c,
          (m, name));
  c->set_particle_quads(t);
  P::operator=(c);
}


IMP_END_NAMESPACE

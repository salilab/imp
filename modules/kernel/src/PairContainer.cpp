/**
 *  \file PairContainer.cpp   \brief Container for pair.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/PairContainer.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/InternalListPairContainer.h"
#include "IMP/PairModifier.h"
#include "IMP/internal/container_helpers.h"

IMP_BEGIN_NAMESPACE


PairContainer::PairContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
PairContainer::~PairContainer(){
}

PairContainerAdaptor
::PairContainerAdaptor(PairContainer *c): P(c){}
PairContainerAdaptor
::PairContainerAdaptor(const ParticlePairsTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListPairContainer, c,
          (m, name));
  c->set_particle_pairs(t);
  P::operator=(c);
}


IMP_END_NAMESPACE

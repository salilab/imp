/**
 *  \file PairContainer.cpp   \brief Container for pair.
 *
 *  WARNING This file was generated from NAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/PairContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/internal/InternalListPairContainer.h"
#include "IMP/kernel/PairModifier.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/pair_macros.h"

IMPKERNEL_BEGIN_NAMESPACE


PairContainer::PairContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
PairContainer::~PairContainer(){
}

ParticlePairsTemp PairContainer
::get_particle_pairs() const {
  return IMP::kernel::internal::get_particle(get_model(),
                                     get_indexes());
}

#if IMP_USE_DEPRECATED
bool PairContainer
::get_contains_particle_pair(ParticlePair v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  ParticleIndexPair iv= IMP::kernel::internal::get_index(v);
  IMP_FOREACH_PAIR_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

unsigned int PairContainer
::get_number_of_particle_pairs() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}

ParticlePair PairContainer
::get_particle_pair(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

bool PairContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void PairContainer
::apply_generic(const PairModifier *m) const {
  apply(m);
}

void PairContainer
::apply(const PairModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


PairContainerAdaptor
::PairContainerAdaptor(PairContainer *c): P(c){}
PairContainerAdaptor
::PairContainerAdaptor(const ParticlePairsTemp &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListPairContainer, c,
          (m, name));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}

IMPKERNEL_END_NAMESPACE

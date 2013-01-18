/**
 *  \file ListPairContainer.cpp   \brief A list of ParticlePairsTemp.
 *
 *  WARNING This file was generated from ListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListPairContainer.h"
#include "IMP/PairModifier.h"
#include "IMP/PairScore.h"
#include <IMP/internal/InternalListPairContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListPairContainer
::ListPairContainer(const ParticlePairsTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]),
    name)
{
  set_particle_pairs(ps);
}

ListPairContainer
::ListPairContainer(Model *m,
                         const ParticleIndexPairs &ps,
                         std::string name):
  P(m, name)
{
  set(ps);
}

ListPairContainer
::ListPairContainer(Model *m, std::string name):
  P(m, name){
}

ListPairContainer
::ListPairContainer(Model *m, const char *name):
  P(m, name){
}

void ListPairContainer
::add_particle_pair(const ParticlePair& vt) {
  add(IMP::internal::get_index(vt));
}
void ListPairContainer
::add_particle_pairs(const ParticlePairsTemp &c) {
  add(IMP::internal::get_index(c));
}
void ListPairContainer
::set_particle_pairs(const ParticlePairsTemp& c) {
  set(IMP::internal::get_index(c));
}
void ListPairContainer
::set_particle_pairs(const ParticleIndexPairs& c) {
  set(c);
}
void ListPairContainer
::clear_particle_pairs() {
  clear();
}

IMPCONTAINER_END_NAMESPACE

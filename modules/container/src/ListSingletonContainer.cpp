/**
 *  \file ListSingletonContainer.cpp   \brief A list of ParticlesTemp.
 *
 *  WARNING This file was generated from ListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListSingletonContainer.h"
#include "IMP/SingletonModifier.h"
#include "IMP/SingletonScore.h"
#include <IMP/internal/InternalListSingletonContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListSingletonContainer
::ListSingletonContainer(const ParticlesTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]),
    name)
{
  set_particles(ps);
}

ListSingletonContainer
::ListSingletonContainer(const ParticleIndexes &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]),
    name)
{
  set(ps);
}

ListSingletonContainer
::ListSingletonContainer(const ParticlesTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]), name)
{
  set_particles(ps);
}

ListSingletonContainer
::ListSingletonContainer(Model *m, std::string name):
  P(m, name){
}

ListSingletonContainer
::ListSingletonContainer(Model *m, const char *name):
  P(m, name){
}

void ListSingletonContainer
::add_particle(Particle* vt) {
  add(IMP::internal::get_index(vt));
}
void ListSingletonContainer
::add_particles(const ParticlesTemp &c) {
  add(IMP::internal::get_index(c));
}
void ListSingletonContainer
::set_particles(ParticlesTemp c) {
  set(IMP::internal::get_index(c));
}
void ListSingletonContainer
::set_particles(const ParticleIndexes& c) {
  set(s);
}
void ListSingletonContainer
::clear_particles() {
  clear();
}

IMPCONTAINER_END_NAMESPACE

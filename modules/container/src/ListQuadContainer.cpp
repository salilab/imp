/**
 *  \file ListQuadContainer.cpp   \brief A list of ParticleQuadsTemp.
 *
 *  WARNING This file was generated from ListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListQuadContainer.h"
#include "IMP/QuadModifier.h"
#include "IMP/QuadScore.h"
#include <IMP/internal/InternalListQuadContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListQuadContainer
::ListQuadContainer(const ParticleQuadsTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]),
    name)
{
  set_particle_quads(ps);
}

ListQuadContainer
::ListQuadContainer(const ParticleIndexQuads &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]),
    name)
{
  set(ps);
}

ListQuadContainer
::ListQuadContainer(const ParticleQuadsTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]), name)
{
  set_particle_quads(ps);
}

ListQuadContainer
::ListQuadContainer(Model *m, std::string name):
  P(m, name){
}

ListQuadContainer
::ListQuadContainer(Model *m, const char *name):
  P(m, name){
}

void ListQuadContainer
::add_particle_quad(const ParticleQuad& vt) {
  add(IMP::internal::get_index(vt));
}
void ListQuadContainer
::add_particle_quads(const ParticleQuadsTemp &c) {
  add(IMP::internal::get_index(c));
}
void ListQuadContainer
::set_particle_quads(ParticleQuadsTemp c) {
  set(IMP::internal::get_index(c));
}
void ListQuadContainer
::set_particle_quads(const ParticleIndexQuads& c) {
  set(s);
}
void ListQuadContainer
::clear_particle_quads() {
  clear();
}

IMPCONTAINER_END_NAMESPACE

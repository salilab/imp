/**
 *  \file ListTripletContainer.cpp   \brief A list of ParticleTripletsTemp.
 *
 *  WARNING This file was generated from ListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListTripletContainer.h"
#include "IMP/TripletModifier.h"
#include "IMP/TripletScore.h"
#include <IMP/internal/InternalListTripletContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListTripletContainer
::ListTripletContainer(const ParticleTripletsTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]),
    name)
{
  set_particle_triplets(ps);
}

ListTripletContainer
::ListTripletContainer(Model *m,
                         const ParticleIndexTriplets &ps,
                         std::string name):
  P(m, name)
{
  set(ps);
}

ListTripletContainer
::ListTripletContainer(Model *m, std::string name):
  P(m, name){
}

ListTripletContainer
::ListTripletContainer(Model *m, const char *name):
  P(m, name){
}

void ListTripletContainer
::add_particle_triplet(const ParticleTriplet& vt) {
  add(IMP::internal::get_index(vt));
}
void ListTripletContainer
::add_particle_triplets(const ParticleTripletsTemp &c) {
  add(IMP::internal::get_index(c));
}
void ListTripletContainer
::set_particle_triplets(const ParticleTripletsTemp& c) {
  set(IMP::internal::get_index(c));
}
void ListTripletContainer
::set_particle_triplets(const ParticleIndexTriplets& c) {
  set(c);
}
void ListTripletContainer
::clear_particle_triplets() {
  clear();
}

IMPCONTAINER_END_NAMESPACE

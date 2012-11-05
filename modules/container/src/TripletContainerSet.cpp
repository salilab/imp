/**
 *  \file TripletContainerSet.cpp
 *  \brief A set of TripletContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/TripletContainerSet.h"
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

TripletContainerSet
::TripletContainerSet(Model *m, std::string name):
  TripletContainer(m, name) {
}


TripletContainerSet
::TripletContainerSet(const TripletContainersTemp& in,
                        std::string name):
    TripletContainer(IMP::internal::get_model(in), name){
  set_triplet_containers(in);
}

void TripletContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_particle_triplets()
      << " containers" << std::endl;
}


ParticleIndexTriplets TripletContainerSet::get_indexes() const {
  ParticleIndexTriplets sum;
  for (TripletContainerConstIterator it= triplet_containers_begin();
       it != triplet_containers_end(); ++it) {
    ParticleIndexTriplets cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

ParticleIndexTriplets TripletContainerSet::get_all_possible_indexes() const {
  ParticleIndexTriplets sum;
  for (TripletContainerConstIterator it= triplet_containers_begin();
       it != triplet_containers_end(); ++it) {
    ParticleIndexTriplets cur=(*it)->get_all_possible_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(TripletContainerSet,
              TripletContainer,
              triplet_container,
              TripletContainer*,
              TripletContainers);


void TripletContainerSet::apply(const TripletModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    get_triplet_container(i)->apply(sm);
  }
}

void TripletContainerSet::apply(const TripletDerivativeModifier *sm,
                               DerivativeAccumulator &da) const {
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    get_triplet_container(i)->apply(sm, da);
  }
}

double TripletContainerSet::evaluate(const TripletScore *s,
                                       DerivativeAccumulator *da) const {
  return template_evaluate(s, da);
}

double TripletContainerSet::evaluate_if_good(const TripletScore *s,
                                               DerivativeAccumulator *da,
                                               double max) const {
  return template_evaluate_if_good(s, da, max);
}


ParticlesTemp TripletContainerSet::get_all_possible_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    ParticlesTemp cur= get_triplet_container(i)
        ->get_all_possible_particles();
    ret+=cur;
  }
  return ret;
}

bool TripletContainerSet::get_is_changed() const {
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    if (get_triplet_container(i)->get_is_changed()) return true;
  }
  return Container::get_is_changed();
}


ContainersTemp TripletContainerSet::get_input_containers() const {
  return ContainersTemp(triplet_containers_begin(),
                        triplet_containers_end());
}
void TripletContainerSet::do_before_evaluate() {
}

IMPCONTAINER_END_NAMESPACE

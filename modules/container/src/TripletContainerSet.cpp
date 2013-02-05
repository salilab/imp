/**
 *  \file TripletContainerSet.cpp
 *  \brief A set of TripletContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    TripletContainer(IMP::kernel::internal::get_model(in), name){
  set_triplet_containers(in);
}

void TripletContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_triplet_containers()
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

ParticleIndexTriplets TripletContainerSet::get_range_indexes() const {
  ParticleIndexTriplets sum;
  for (TripletContainerConstIterator it= triplet_containers_begin();
       it != triplet_containers_end(); ++it) {
    ParticleIndexTriplets cur=(*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(TripletContainerSet,
              TripletContainer,
              triplet_container,
              TripletContainer*,
              TripletContainers);


void TripletContainerSet::do_apply(const TripletModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    get_triplet_container(i)->apply(sm);
  }
}

ParticleIndexes TripletContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    ret+= get_triplet_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

void TripletContainerSet::do_before_evaluate() {
  for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
    if (get_triplet_container(i)->get_is_changed()) {
      set_is_changed(true);
      return;
    }
  }
  set_is_changed(false);
}

ModelObjectsTemp TripletContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(triplet_containers_begin(),
                        triplet_containers_end());
}

IMPCONTAINER_END_NAMESPACE

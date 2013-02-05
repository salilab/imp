/**
 *  \file SingletonContainerSet.cpp
 *  \brief A set of SingletonContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/SingletonContainerSet.h"
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

SingletonContainerSet
::SingletonContainerSet(Model *m, std::string name):
  SingletonContainer(m, name) {
}


SingletonContainerSet
::SingletonContainerSet(const SingletonContainersTemp& in,
                        std::string name):
    SingletonContainer(IMP::kernel::internal::get_model(in), name){
  set_singleton_containers(in);
}

void SingletonContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_singleton_containers()
      << " containers" << std::endl;
}


ParticleIndexes SingletonContainerSet::get_indexes() const {
  ParticleIndexes sum;
  for (SingletonContainerConstIterator it= singleton_containers_begin();
       it != singleton_containers_end(); ++it) {
    ParticleIndexes cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

ParticleIndexes SingletonContainerSet::get_range_indexes() const {
  ParticleIndexes sum;
  for (SingletonContainerConstIterator it= singleton_containers_begin();
       it != singleton_containers_end(); ++it) {
    ParticleIndexes cur=(*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(SingletonContainerSet,
              SingletonContainer,
              singleton_container,
              SingletonContainer*,
              SingletonContainers);


void SingletonContainerSet::do_apply(const SingletonModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_singleton_containers(); ++i) {
    get_singleton_container(i)->apply(sm);
  }
}

ParticleIndexes SingletonContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_singleton_containers(); ++i) {
    ret+= get_singleton_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

void SingletonContainerSet::do_before_evaluate() {
  for (unsigned int i=0; i< get_number_of_singleton_containers(); ++i) {
    if (get_singleton_container(i)->get_is_changed()) {
      set_is_changed(true);
      return;
    }
  }
  set_is_changed(false);
}

ModelObjectsTemp SingletonContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(singleton_containers_begin(),
                        singleton_containers_end());
}

IMPCONTAINER_END_NAMESPACE

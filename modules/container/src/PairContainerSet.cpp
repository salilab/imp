/**
 *  \file PairContainerSet.cpp
 *  \brief A set of PairContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/PairContainerSet.h"
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

PairContainerSet
::PairContainerSet(Model *m, std::string name):
  PairContainer(m, name) {
}


PairContainerSet
::PairContainerSet(const PairContainersTemp& in,
                        std::string name):
    PairContainer(IMP::kernel::internal::get_model(in), name){
  set_pair_containers(in);
}

void PairContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_pair_containers()
      << " containers" << std::endl;
}


ParticleIndexPairs PairContainerSet::get_indexes() const {
  ParticleIndexPairs sum;
  for (PairContainerConstIterator it= pair_containers_begin();
       it != pair_containers_end(); ++it) {
    ParticleIndexPairs cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

ParticleIndexPairs PairContainerSet::get_range_indexes() const {
  ParticleIndexPairs sum;
  for (PairContainerConstIterator it= pair_containers_begin();
       it != pair_containers_end(); ++it) {
    ParticleIndexPairs cur=(*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(PairContainerSet,
              PairContainer,
              pair_container,
              PairContainer*,
              PairContainers);


void PairContainerSet::do_apply(const PairModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    get_pair_container(i)->apply(sm);
  }
}

ParticleIndexes PairContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    ret+= get_pair_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

void PairContainerSet::do_before_evaluate() {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    if (get_pair_container(i)->get_is_changed()) {
      set_is_changed(true);
      return;
    }
  }
  set_is_changed(false);
}

ModelObjectsTemp PairContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(pair_containers_begin(),
                        pair_containers_end());
}

IMPCONTAINER_END_NAMESPACE

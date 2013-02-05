/**
 *  \file QuadContainerSet.cpp
 *  \brief A set of QuadContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/QuadContainerSet.h"
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

QuadContainerSet
::QuadContainerSet(Model *m, std::string name):
  QuadContainer(m, name) {
}


QuadContainerSet
::QuadContainerSet(const QuadContainersTemp& in,
                        std::string name):
    QuadContainer(IMP::kernel::internal::get_model(in), name){
  set_quad_containers(in);
}

void QuadContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_quad_containers()
      << " containers" << std::endl;
}


ParticleIndexQuads QuadContainerSet::get_indexes() const {
  ParticleIndexQuads sum;
  for (QuadContainerConstIterator it= quad_containers_begin();
       it != quad_containers_end(); ++it) {
    ParticleIndexQuads cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

ParticleIndexQuads QuadContainerSet::get_range_indexes() const {
  ParticleIndexQuads sum;
  for (QuadContainerConstIterator it= quad_containers_begin();
       it != quad_containers_end(); ++it) {
    ParticleIndexQuads cur=(*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(QuadContainerSet,
              QuadContainer,
              quad_container,
              QuadContainer*,
              QuadContainers);


void QuadContainerSet::do_apply(const QuadModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    get_quad_container(i)->apply(sm);
  }
}

ParticleIndexes QuadContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    ret+= get_quad_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

void QuadContainerSet::do_before_evaluate() {
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    if (get_quad_container(i)->get_is_changed()) {
      set_is_changed(true);
      return;
    }
  }
  set_is_changed(false);
}

ModelObjectsTemp QuadContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(quad_containers_begin(),
                        quad_containers_end());
}

IMPCONTAINER_END_NAMESPACE

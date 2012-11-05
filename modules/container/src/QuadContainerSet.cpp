/**
 *  \file QuadContainerSet.cpp
 *  \brief A set of QuadContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
    QuadContainer(IMP::internal::get_model(in), name){
  set_quad_containers(in);
}

void QuadContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_particle_quads()
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

ParticleIndexQuads QuadContainerSet::get_all_possible_indexes() const {
  ParticleIndexQuads sum;
  for (QuadContainerConstIterator it= quad_containers_begin();
       it != quad_containers_end(); ++it) {
    ParticleIndexQuads cur=(*it)->get_all_possible_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(QuadContainerSet,
              QuadContainer,
              quad_container,
              QuadContainer*,
              QuadContainers);


void QuadContainerSet::apply(const QuadModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    get_quad_container(i)->apply(sm);
  }
}

void QuadContainerSet::apply(const QuadDerivativeModifier *sm,
                               DerivativeAccumulator &da) const {
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    get_quad_container(i)->apply(sm, da);
  }
}

double QuadContainerSet::evaluate(const QuadScore *s,
                                       DerivativeAccumulator *da) const {
  return template_evaluate(s, da);
}

double QuadContainerSet::evaluate_if_good(const QuadScore *s,
                                               DerivativeAccumulator *da,
                                               double max) const {
  return template_evaluate_if_good(s, da, max);
}


ParticlesTemp QuadContainerSet::get_all_possible_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    ParticlesTemp cur= get_quad_container(i)
        ->get_all_possible_particles();
    ret+=cur;
  }
  return ret;
}

bool QuadContainerSet::get_is_changed() const {
  for (unsigned int i=0; i< get_number_of_quad_containers(); ++i) {
    if (get_quad_container(i)->get_is_changed()) return true;
  }
  return Container::get_is_changed();
}


ContainersTemp QuadContainerSet::get_input_containers() const {
  return ContainersTemp(quad_containers_begin(),
                        quad_containers_end());
}
void QuadContainerSet::do_before_evaluate() {
}

IMPCONTAINER_END_NAMESPACE

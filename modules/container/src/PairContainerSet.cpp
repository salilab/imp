/**
 *  \file PairContainerSet.cpp
 *  \brief A set of PairContainers.
 *
 *  WARNING This file was generated from NAMEContainerSet.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
    PairContainer(IMP::internal::get_model(in), name){
  set_pair_containers(in);
}

void PairContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_particle_pairs()
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

ParticleIndexPairs PairContainerSet::get_all_possible_indexes() const {
  ParticleIndexPairs sum;
  for (PairContainerConstIterator it= pair_containers_begin();
       it != pair_containers_end(); ++it) {
    ParticleIndexPairs cur=(*it)->get_all_possible_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(PairContainerSet,
              PairContainer,
              pair_container,
              PairContainer*,
              PairContainers);


void PairContainerSet::apply(const PairModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    get_pair_container(i)->apply(sm);
  }
}

void PairContainerSet::apply(const PairDerivativeModifier *sm,
                               DerivativeAccumulator &da) const {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    get_pair_container(i)->apply(sm, da);
  }
}

double PairContainerSet::evaluate(const PairScore *s,
                                       DerivativeAccumulator *da) const {
  return template_evaluate(s, da);
}

double PairContainerSet::evaluate_if_good(const PairScore *s,
                                               DerivativeAccumulator *da,
                                               double max) const {
  return template_evaluate_if_good(s, da, max);
}


ParticlesTemp PairContainerSet::get_all_possible_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    ParticlesTemp cur= get_pair_container(i)
        ->get_all_possible_particles();
    ret+=cur;
  }
  return ret;
}

bool PairContainerSet::get_is_changed() const {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    if (get_pair_container(i)->get_is_changed()) return true;
  }
  return Container::get_is_changed();
}


ContainersTemp PairContainerSet::get_input_containers() const {
  return ContainersTemp(pair_containers_begin(),
                        pair_containers_end());
}
void PairContainerSet::do_before_evaluate() {
}

IMPCONTAINER_END_NAMESPACE

/**
 *  \file CLASSNAMEContainerSet.cpp
 *  \brief A set of CLASSNAMEContainers.
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/CLASSNAMEContainerSet.h"
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

CLASSNAMEContainerSet
::CLASSNAMEContainerSet(Model *m, std::string name):
  CLASSNAMEContainer(m, name) {
}


CLASSNAMEContainerSet
::CLASSNAMEContainerSet(const CLASSNAMEContainersTemp& in,
                        std::string name):
    CLASSNAMEContainer(IMP::internal::get_model(in), name){
  set_CLASSFUNCTIONNAME_containers(in);
}

void CLASSNAMEContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_CLASSFUNCTIONNAME_containers()
      << " containers" << std::endl;
}


PLURALINDEXTYPE CLASSNAMEContainerSet::get_indexes() const {
  PLURALINDEXTYPE sum;
  for (CLASSNAMEContainerConstIterator it= CLASSFUNCTIONNAME_containers_begin();
       it != CLASSFUNCTIONNAME_containers_end(); ++it) {
    PLURALINDEXTYPE cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

PLURALINDEXTYPE CLASSNAMEContainerSet::get_range_indexes() const {
  PLURALINDEXTYPE sum;
  for (CLASSNAMEContainerConstIterator it= CLASSFUNCTIONNAME_containers_begin();
       it != CLASSFUNCTIONNAME_containers_end(); ++it) {
    PLURALINDEXTYPE cur=(*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(CLASSNAMEContainerSet,
              CLASSNAMEContainer,
              CLASSFUNCTIONNAME_container,
              CLASSNAMEContainer*,
              CLASSNAMEContainers);


void CLASSNAMEContainerSet::apply(const CLASSNAMEModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    get_CLASSFUNCTIONNAME_container(i)->apply(sm);
  }
}

void CLASSNAMEContainerSet::apply(const CLASSNAMEDerivativeModifier *sm,
                               DerivativeAccumulator &da) const {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    get_CLASSFUNCTIONNAME_container(i)->apply(sm, da);
  }
}

double CLASSNAMEContainerSet::evaluate(const CLASSNAMEScore *s,
                                       DerivativeAccumulator *da) const {
  return template_evaluate(s, da);
}

double CLASSNAMEContainerSet::evaluate_if_good(const CLASSNAMEScore *s,
                                               DerivativeAccumulator *da,
                                               double max) const {
  return template_evaluate_if_good(s, da, max);
}


ParticleIndexes CLASSNAMEContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    ret+= get_CLASSFUNCTIONNAME_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

bool CLASSNAMEContainerSet::get_is_changed() const {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    if (get_CLASSFUNCTIONNAME_container(i)->get_is_changed()) return true;
  }
  return Container::get_is_changed();
}


ModelObjectsTemp CLASSNAMEContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(CLASSFUNCTIONNAME_containers_begin(),
                        CLASSFUNCTIONNAME_containers_end());
}
void CLASSNAMEContainerSet::do_before_evaluate() {
}

IMPCONTAINER_END_NAMESPACE

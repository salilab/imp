/**
 *  \file CLASSNAMEContainerSet.cpp
 *  \brief A set of CLASSNAMEContainers.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    CLASSNAMEContainer(IMP::kernel::internal::get_model(in), name){
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


void CLASSNAMEContainerSet::do_apply(const CLASSNAMEModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    get_CLASSFUNCTIONNAME_container(i)->apply(sm);
  }
}

ParticleIndexes CLASSNAMEContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    ret+= get_CLASSFUNCTIONNAME_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

void CLASSNAMEContainerSet::do_before_evaluate() {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    if (get_CLASSFUNCTIONNAME_container(i)->get_is_changed()) {
      set_is_changed(true);
      return;
    }
  }
  set_is_changed(false);
}

ModelObjectsTemp CLASSNAMEContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(CLASSFUNCTIONNAME_containers_begin(),
                        CLASSFUNCTIONNAME_containers_end());
}

IMPCONTAINER_END_NAMESPACE

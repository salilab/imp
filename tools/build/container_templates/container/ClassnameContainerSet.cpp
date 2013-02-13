/**
 *  \file ClassnameContainerSet.cpp
 *  \brief A set of ClassnameContainers.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ClassnameContainerSet.h"
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

ClassnameContainerSet
::ClassnameContainerSet(Model *m, std::string name):
  ClassnameContainer(m, name) {
}


ClassnameContainerSet
::ClassnameContainerSet(const ClassnameContainersTemp& in,
                        std::string name):
    ClassnameContainer(IMP::kernel::internal::get_model(in), name){
  set_CLASSFUNCTIONNAME_containers(in);
}

void ClassnameContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_CLASSFUNCTIONNAME_containers()
      << " containers" << std::endl;
}


PLURALINDEXTYPE ClassnameContainerSet::get_indexes() const {
  PLURALINDEXTYPE sum;
  for (ClassnameContainerConstIterator it= CLASSFUNCTIONNAME_containers_begin();
       it != CLASSFUNCTIONNAME_containers_end(); ++it) {
    PLURALINDEXTYPE cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

PLURALINDEXTYPE ClassnameContainerSet::get_range_indexes() const {
  PLURALINDEXTYPE sum;
  for (ClassnameContainerConstIterator it= CLASSFUNCTIONNAME_containers_begin();
       it != CLASSFUNCTIONNAME_containers_end(); ++it) {
    PLURALINDEXTYPE cur=(*it)->get_range_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(ClassnameContainerSet,
              ClassnameContainer,
              CLASSFUNCTIONNAME_container,
              ClassnameContainer*,
              ClassnameContainers);


void ClassnameContainerSet::do_apply(const ClassnameModifier *sm) const {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    get_CLASSFUNCTIONNAME_container(i)->apply(sm);
  }
}

ParticleIndexes ClassnameContainerSet::get_all_possible_indexes() const {
  ParticleIndexes ret;
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    ret+= get_CLASSFUNCTIONNAME_container(i)
        ->get_all_possible_indexes();
  }
  return ret;
}

void ClassnameContainerSet::do_before_evaluate() {
  for (unsigned int i=0; i< get_number_of_CLASSFUNCTIONNAME_containers(); ++i) {
    if (get_CLASSFUNCTIONNAME_container(i)->get_is_changed()) {
      set_is_changed(true);
      return;
    }
  }
  set_is_changed(false);
}

ModelObjectsTemp ClassnameContainerSet::do_get_inputs() const {
  return ModelObjectsTemp(CLASSFUNCTIONNAME_containers_begin(),
                        CLASSFUNCTIONNAME_containers_end());
}

IMPCONTAINER_END_NAMESPACE

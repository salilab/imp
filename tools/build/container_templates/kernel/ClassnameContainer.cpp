/**
 *  \file ClassnameContainer.cpp   \brief Container for classname.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/ClassnameContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/internal/InternalListClassnameContainer.h"
#include "IMP/kernel/ClassnameModifier.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/classname_macros.h"

IMPKERNEL_BEGIN_NAMESPACE


ClassnameContainer::ClassnameContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
ClassnameContainer::~ClassnameContainer(){
}

PLURALVARIABLETYPE ClassnameContainer
::get_FUNCTIONNAMEs() const {
  return IMP::kernel::internal::get_particle(get_model(),
                                     get_indexes());
}

#if IMP_USE_DEPRECATED
bool ClassnameContainer
::get_contains_FUNCTIONNAME(VARIABLETYPE v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  INDEXTYPE iv= IMP::kernel::internal::get_index(v);
  IMP_FOREACH_CLASSNAME_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

unsigned int ClassnameContainer
::get_number_of_FUNCTIONNAMEs() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}

VARIABLETYPE ClassnameContainer
::get_FUNCTIONNAME(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

bool ClassnameContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void ClassnameContainer
::apply_generic(const ClassnameModifier *m) const {
  apply(m);
}

void ClassnameContainer
::apply(const ClassnameModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


ClassnameContainerAdaptor
::ClassnameContainerAdaptor(ClassnameContainer *c): P(c){}
ClassnameContainerAdaptor
::ClassnameContainerAdaptor(const PLURALVARIABLETYPE &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListClassnameContainer, c,
          (m, name));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}

IMPKERNEL_END_NAMESPACE

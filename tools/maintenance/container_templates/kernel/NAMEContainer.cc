/**
 *  \file CLASSNAMEContainer.cpp   \brief Container for LCCLASSNAME.
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/CLASSNAMEContainer.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/InternalListCLASSNAMEContainer.h"
#include "IMP/CLASSNAMEModifier.h"
#include "IMP/internal/container_helpers.h"
#include "IMP/LCCLASSNAME_macros.h"

IMP_BEGIN_NAMESPACE


CLASSNAMEContainer::CLASSNAMEContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
CLASSNAMEContainer::~CLASSNAMEContainer(){
}

#if IMP_USE_DEPRECATED
bool CLASSNAMEContainer
::get_contains_FUNCTIONNAME(VARIABLETYPE v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  INDEXTYPE iv= IMP::internal::get_index(v);
  IMP_FOREACH_HEADERNAME_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

PLURALVARIABLETYPE CLASSNAMEContainer
::get_FUNCTIONNAMEs() const {
  return IMP::internal::get_particle(get_model(),
                                     get_indexes());
}

unsigned int CLASSNAMEContainer
::get_number_of_FUNCTIONNAMEs() const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get_number();
}
VARIABLETYPE CLASSNAMEContainer
::get_FUNCTIONNAME(unsigned int i) const {
  IMP_DEPRECATED_FUNCTION(IMP_CONTAINER_FOREACH());
  return get(i);
}
#endif

CLASSNAMEContainerAdaptor
::CLASSNAMEContainerAdaptor(CLASSNAMEContainer *c): P(c){}
CLASSNAMEContainerAdaptor
::CLASSNAMEContainerAdaptor(const PLURALVARIABLETYPE &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListCLASSNAMEContainer, c,
          (m, name));
  c->set(IMP::internal::get_index(t));
  P::operator=(c);
}

IMP_END_NAMESPACE

/**
 *  \file CLASSNAMEContainer.cpp   \brief Container for LCCLASSNAME.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/CLASSNAMEContainer.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/internal/InternalListCLASSNAMEContainer.h"
#include "IMP/kernel/CLASSNAMEModifier.h"
#include "IMP/kernel/internal/container_helpers.h"
#include "IMP/kernel/LCCLASSNAME_macros.h"

IMPKERNEL_BEGIN_NAMESPACE


CLASSNAMEContainer::CLASSNAMEContainer(Model *m, std::string name):
  Container(m,name){
}

// here for gcc
CLASSNAMEContainer::~CLASSNAMEContainer(){
}

PLURALVARIABLETYPE CLASSNAMEContainer
::get_FUNCTIONNAMEs() const {
  return IMP::kernel::internal::get_particle(get_model(),
                                     get_indexes());
}

#if IMP_USE_DEPRECATED
bool CLASSNAMEContainer
::get_contains_FUNCTIONNAME(VARIABLETYPE v) const {
  IMP_DEPRECATED_FUNCTION(something else);
  INDEXTYPE iv= IMP::kernel::internal::get_index(v);
  IMP_FOREACH_HEADERNAME_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
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

bool CLASSNAMEContainer
::get_provides_access() const {
  validate_readable();
  return do_get_provides_access();
}

void CLASSNAMEContainer
::apply_generic(const CLASSNAMEModifier *m) const {
  apply(m);
}

void CLASSNAMEContainer
::apply(const CLASSNAMEModifier *sm) const {
  validate_readable();
  do_apply(sm);
}


CLASSNAMEContainerAdaptor
::CLASSNAMEContainerAdaptor(CLASSNAMEContainer *c): P(c){}
CLASSNAMEContainerAdaptor
::CLASSNAMEContainerAdaptor(const PLURALVARIABLETYPE &t,
                                                 std::string name) {
  Model *m=internal::get_model(t);
  IMP_NEW(internal::InternalListCLASSNAMEContainer, c,
          (m, name));
  c->set(IMP::kernel::internal::get_index(t));
  P::operator=(c);
}

IMPKERNEL_END_NAMESPACE

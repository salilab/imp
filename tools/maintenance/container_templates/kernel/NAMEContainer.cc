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

bool CLASSNAMEContainer
::get_contains_FUNCTIONNAME(VARIABLETYPE v) const {
  INDEXTYPE iv= IMP::internal::get_index(v);
  IMP_FOREACH_HEADERNAME_INDEX(this, {
      if (_1 == iv) return true;
    });
  return false;
}

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

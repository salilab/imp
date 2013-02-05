/**
 *  \file ListCLASSNAMEContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListCLASSNAMEContainer.h"
#include "IMP/CLASSNAMEModifier.h"
#include "IMP/CLASSNAMEScore.h"
#include <IMP/internal/InternalListCLASSNAMEContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListCLASSNAMEContainer
::DynamicListCLASSNAMEContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListCLASSNAMEContainer
::add_FUNCTIONNAME(ARGUMENTTYPE vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListCLASSNAMEContainer
::add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListCLASSNAMEContainer
::set_FUNCTIONNAMEs(PLURALVARIABLETYPE c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListCLASSNAMEContainer
::clear_FUNCTIONNAMEs() {
  clear();
}
IMPCONTAINER_END_NAMESPACE

/**
 *  \file ListClassnameContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListClassnameContainer.h"
#include "IMP/ClassnameModifier.h"
#include "IMP/ClassnameScore.h"
#include <IMP/internal/InternalListClassnameContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListClassnameContainer
::DynamicListClassnameContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListClassnameContainer
::add_FUNCTIONNAME(ARGUMENTTYPE vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void DynamicListClassnameContainer
::add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c) {
  add(IMP::kernel::internal::get_index(c));
}
void DynamicListClassnameContainer
::set_FUNCTIONNAMEs(PLURALVARIABLETYPE c) {
  set(IMP::kernel::internal::get_index(c));
}
void DynamicListClassnameContainer
::clear_FUNCTIONNAMEs() {
  clear();
}
IMPCONTAINER_END_NAMESPACE

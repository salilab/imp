/**
 *  \file ListClassnameContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListClassnameContainer.h"
#include "IMP/ClassnameModifier.h"
#include "IMP/ClassnameScore.h"
#include <IMP/internal/InternalListClassnameContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListClassnameContainer
::ListClassnameContainer(const PLURALVARIABLETYPE &ps,
                         std::string name):
  P(IMP::kernel::internal::get_model(ps[0]),
    name)
{
  set_FUNCTIONNAMEs(ps);
}

ListClassnameContainer
::ListClassnameContainer(Model *m,
                         const PLURALINDEXTYPE &ps,
                         std::string name):
  P(m, name)
{
  set(ps);
}

ListClassnameContainer
::ListClassnameContainer(Model *m, std::string name):
  P(m, name){
}

ListClassnameContainer
::ListClassnameContainer(Model *m, const char *name):
  P(m, name){
}

void ListClassnameContainer
::add_FUNCTIONNAME(ARGUMENTTYPE vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void ListClassnameContainer
::add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c) {
  add(IMP::kernel::internal::get_index(c));
}
void ListClassnameContainer
::set_FUNCTIONNAMEs(const PLURALVARIABLETYPE& c) {
  set(IMP::kernel::internal::get_index(c));
}
void ListClassnameContainer
::set_FUNCTIONNAMEs(const PLURALINDEXTYPE& c) {
  set(c);
}
void ListClassnameContainer
::clear_FUNCTIONNAMEs() {
  clear();
}

IMPCONTAINER_END_NAMESPACE

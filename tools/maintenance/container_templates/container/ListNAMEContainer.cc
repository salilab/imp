/**
 *  \file ListCLASSNAMEContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListCLASSNAMEContainer.h"
#include "IMP/CLASSNAMEModifier.h"
#include "IMP/CLASSNAMEScore.h"
#include <IMP/internal/InternalListCLASSNAMEContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListCLASSNAMEContainer
::ListCLASSNAMEContainer(const PLURALVARIABLETYPE &ps,
                         std::string name):
  P(IMP::kernel::internal::get_model(ps[0]),
    name)
{
  set_FUNCTIONNAMEs(ps);
}

ListCLASSNAMEContainer
::ListCLASSNAMEContainer(Model *m,
                         const PLURALINDEXTYPE &ps,
                         std::string name):
  P(m, name)
{
  set(ps);
}

ListCLASSNAMEContainer
::ListCLASSNAMEContainer(Model *m, std::string name):
  P(m, name){
}

ListCLASSNAMEContainer
::ListCLASSNAMEContainer(Model *m, const char *name):
  P(m, name){
}

void ListCLASSNAMEContainer
::add_FUNCTIONNAME(ARGUMENTTYPE vt) {
  add(IMP::kernel::internal::get_index(vt));
}
void ListCLASSNAMEContainer
::add_FUNCTIONNAMEs(const PLURALVARIABLETYPE &c) {
  add(IMP::kernel::internal::get_index(c));
}
void ListCLASSNAMEContainer
::set_FUNCTIONNAMEs(const PLURALVARIABLETYPE& c) {
  set(IMP::kernel::internal::get_index(c));
}
void ListCLASSNAMEContainer
::set_FUNCTIONNAMEs(const PLURALINDEXTYPE& c) {
  set(c);
}
void ListCLASSNAMEContainer
::clear_FUNCTIONNAMEs() {
  clear();
}

IMPCONTAINER_END_NAMESPACE

/**
 *  \file ListCLASSNAMEContainer.h    \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_HEADERNAME_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_HEADERNAME_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikeCLASSNAMEContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalListCLASSNAMEContainer:
  public ListLikeCLASSNAMEContainer
{
  typedef ListLikeCLASSNAMEContainer P;
 public:
  InternalListCLASSNAMEContainer(Model *m, std::string name);
  InternalListCLASSNAMEContainer(Model *m, const char *name);
  void add_FUNCTIONNAME(ARGUMENTTYPE vt) {
    get_model()->reset_dependencies();
    IMP_USAGE_CHECK(IMP::internal::is_valid(vt),
                    "Passed CLASSNAME cannot be nullptr (or None)");

    add_to_list(IMP::internal::get_index(vt));
  }
  void add_FUNCTIONNAME(PASSINDEXTYPE vt) {
    get_model()->reset_dependencies();
    add_to_list(vt);
  }
  void add_FUNCTIONNAMEs(const PLURALARGUMENTTYPE &c) {
    if (c.empty()) return;
    get_model()->reset_dependencies();
    PLURALINDEXTYPE cp= IMP::internal::get_index(c);
    add_to_list(cp);
  }
  void remove_FUNCTIONNAMEs(const PLURALARGUMENTTYPE &c);
  void set_FUNCTIONNAMEs(PLURALARGUMENTTYPE c) {
    get_model()->reset_dependencies();
    PLURALINDEXTYPE cp= IMP::internal::get_index(c);
    update_list(cp);
  }
  void set_FUNCTIONNAMEs(PLURALINDEXTYPE cp) {
    get_model()->reset_dependencies();
    update_list(cp);
  }
  void clear_FUNCTIONNAMEs() {
    get_model()->reset_dependencies();
    PLURALINDEXTYPE t;
    update_list(t);
  }
  IMP_LISTLIKE_HEADERNAME_CONTAINER(InternalListCLASSNAMEContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_HEADERNAME_CONTAINER_H */

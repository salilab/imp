/**
 *  \file ListClassnameContainer.h    \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_CLASSNAME_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_CLASSNAME_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeClassnameContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalListClassnameContainer:
  public ListLikeClassnameContainer
{
  typedef ListLikeClassnameContainer P;
 public:
  InternalListClassnameContainer(Model *m, std::string name);
  InternalListClassnameContainer(Model *m, const char *name);
  void add(PASSINDEXTYPE vt);
  void add(const PLURALINDEXTYPE &c);
  void set(PLURALINDEXTYPE cp);
  void remove(PASSINDEXTYPE vt);
  void clear();
  IMP_LISTLIKE_CLASSNAME_CONTAINER(InternalListClassnameContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_CLASSNAME_CONTAINER_H */

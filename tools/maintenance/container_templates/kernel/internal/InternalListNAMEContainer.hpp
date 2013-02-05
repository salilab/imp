/**
 *  \file ListCLASSNAMEContainer.h    \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_HEADERNAME_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_HEADERNAME_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeCLASSNAMEContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalListCLASSNAMEContainer:
  public ListLikeCLASSNAMEContainer
{
  typedef ListLikeCLASSNAMEContainer P;
 public:
  InternalListCLASSNAMEContainer(Model *m, std::string name);
  InternalListCLASSNAMEContainer(Model *m, const char *name);
  void add(PASSINDEXTYPE vt);
  void add(const PLURALINDEXTYPE &c);
  void set(PLURALINDEXTYPE cp);
  void remove(PASSINDEXTYPE vt);
  void clear();
  IMP_LISTLIKE_HEADERNAME_CONTAINER(InternalListCLASSNAMEContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_HEADERNAME_CONTAINER_H */

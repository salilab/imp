/**
 *  \file ListClassnameContainer.h
 *  \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_CLASSNAME_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_CLASSNAME_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeClassnameContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalDynamicListClassnameContainer:
  public ListLikeClassnameContainer
{
  typedef ListLikeClassnameContainer P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes& cp) const;
 public:
  InternalDynamicListClassnameContainer(Container *m, std::string name);
  InternalDynamicListClassnameContainer(Container *m, const char *name);
  void add(PASSINDEXTYPE vt);
  void add(const PLURALINDEXTYPE &c);
  void set(PLURALINDEXTYPE cp);
  void clear();
  IMP_LISTLIKE_CLASSNAME_CONTAINER(InternalDynamicListClassnameContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_CLASSNAME_CONTAINER_H */

/**
 *  \file ListSingletonContainer.h
 *  \brief Store a list of ParticlesTemp
 *
 *  WARNING This file was generated from InternalDynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_SINGLETON_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_SINGLETON_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeSingletonContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalDynamicListSingletonContainer:
  public ListLikeSingletonContainer
{
  typedef ListLikeSingletonContainer P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes& cp) const;
 public:
  InternalDynamicListSingletonContainer(Container *m, std::string name);
  InternalDynamicListSingletonContainer(Container *m, const char *name);
  void add(ParticleIndex vt);
  void add(const ParticleIndexes &c);
  void set(ParticleIndexes cp);
  void clear();
  IMP_LISTLIKE_SINGLETON_CONTAINER(InternalDynamicListSingletonContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_SINGLETON_CONTAINER_H */

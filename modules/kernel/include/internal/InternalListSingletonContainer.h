/**
 *  \file ListSingletonContainer.h    \brief Store a list of ParticlesTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_SINGLETON_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_SINGLETON_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeSingletonContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalListSingletonContainer:
  public ListLikeSingletonContainer
{
  typedef ListLikeSingletonContainer P;
 public:
  InternalListSingletonContainer(Model *m, std::string name);
  InternalListSingletonContainer(Model *m, const char *name);
  void add(ParticleIndex vt);
  void add(const ParticleIndexes &c);
  void set(ParticleIndexes cp);
  void remove(ParticleIndex vt);
  void clear();
  IMP_LISTLIKE_SINGLETON_CONTAINER(InternalListSingletonContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_SINGLETON_CONTAINER_H */

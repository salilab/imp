/**
 *  \file ListTripletContainer.h
 *  \brief Store a list of ParticleTripletsTemp
 *
 *  WARNING This file was generated from InternalDynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_TRIPLET_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_TRIPLET_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeTripletContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalDynamicListTripletContainer:
  public ListLikeTripletContainer
{
  typedef ListLikeTripletContainer P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes& cp) const;
 public:
  InternalDynamicListTripletContainer(Container *m, std::string name);
  InternalDynamicListTripletContainer(Container *m, const char *name);
  void add(const ParticleIndexTriplet& vt);
  void add(const ParticleIndexTriplets &c);
  void set(ParticleIndexTriplets cp);
  void clear();
  IMP_LISTLIKE_TRIPLET_CONTAINER(InternalDynamicListTripletContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_TRIPLET_CONTAINER_H */

/**
 *  \file ListTripletContainer.h    \brief Store a list of ParticleTripletsTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_TRIPLET_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_TRIPLET_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeTripletContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalListTripletContainer:
  public ListLikeTripletContainer
{
  typedef ListLikeTripletContainer P;
 public:
  InternalListTripletContainer(Model *m, std::string name);
  InternalListTripletContainer(Model *m, const char *name);
  void add(const ParticleIndexTriplet& vt);
  void add(const ParticleIndexTriplets &c);
  void set(ParticleIndexTriplets cp);
  void remove(const ParticleIndexTriplet& vt);
  void clear();
  IMP_LISTLIKE_TRIPLET_CONTAINER(InternalListTripletContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_TRIPLET_CONTAINER_H */

/**
 *  \file ListPairContainer.h
 *  \brief Store a list of ParticlePairsTemp
 *
 *  WARNING This file was generated from InternalDynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_PAIR_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_PAIR_CONTAINER_H

#include "../kernel_config.h"
#include "container_helpers.h"
#include "ListLikePairContainer.h"
#include <IMP/base/Pointer.h>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT InternalDynamicListPairContainer:
  public ListLikePairContainer
{
  typedef ListLikePairContainer P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes& cp) const;
 public:
  InternalDynamicListPairContainer(Container *m, std::string name);
  InternalDynamicListPairContainer(Container *m, const char *name);
  void add(const ParticleIndexPair& vt);
  void add(const ParticleIndexPairs &c);
  void set(ParticleIndexPairs cp);
  void clear();
  IMP_LISTLIKE_PAIR_CONTAINER(InternalDynamicListPairContainer);
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_PAIR_CONTAINER_H */

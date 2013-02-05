/**
 *  \file ListPairContainer.h    \brief Store a list of ParticlePairsTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_PAIR_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_PAIR_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikePairContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalListPairContainer:
  public ListLikePairContainer
{
  typedef ListLikePairContainer P;
 public:
  InternalListPairContainer(Model *m, std::string name);
  InternalListPairContainer(Model *m, const char *name);
  void add(const ParticleIndexPair& vt);
  void add(const ParticleIndexPairs &c);
  void set(ParticleIndexPairs cp);
  void remove(const ParticleIndexPair& vt);
  void clear();
  IMP_LISTLIKE_PAIR_CONTAINER(InternalListPairContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_PAIR_CONTAINER_H */

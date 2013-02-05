/**
 *  \file ListQuadContainer.h
 *  \brief Store a list of ParticleQuadsTemp
 *
 *  WARNING This file was generated from InternalDynamicListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_QUAD_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_QUAD_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeQuadContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalDynamicListQuadContainer:
  public ListLikeQuadContainer
{
  typedef ListLikeQuadContainer P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes& cp) const;
 public:
  InternalDynamicListQuadContainer(Container *m, std::string name);
  InternalDynamicListQuadContainer(Container *m, const char *name);
  void add(const ParticleIndexQuad& vt);
  void add(const ParticleIndexQuads &c);
  void set(ParticleIndexQuads cp);
  void clear();
  IMP_LISTLIKE_QUAD_CONTAINER(InternalDynamicListQuadContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_DYNAMIC_LIST_QUAD_CONTAINER_H */

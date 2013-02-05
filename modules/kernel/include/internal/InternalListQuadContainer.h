/**
 *  \file ListQuadContainer.h    \brief Store a list of ParticleQuadsTemp
 *
 *  WARNING This file was generated from InternalListNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_QUAD_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_QUAD_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeQuadContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


class IMPKERNELEXPORT InternalListQuadContainer:
  public ListLikeQuadContainer
{
  typedef ListLikeQuadContainer P;
 public:
  InternalListQuadContainer(Model *m, std::string name);
  InternalListQuadContainer(Model *m, const char *name);
  void add(const ParticleIndexQuad& vt);
  void add(const ParticleIndexQuads &c);
  void set(ParticleIndexQuads cp);
  void remove(const ParticleIndexQuad& vt);
  void clear();
  IMP_LISTLIKE_QUAD_CONTAINER(InternalListQuadContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_LIST_QUAD_CONTAINER_H */

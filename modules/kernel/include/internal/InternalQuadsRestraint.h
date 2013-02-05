/**
 *  \file CoreQuadsRestraint.h
 *  \brief Apply a QuadScore to each Quad in a list.
 *
 *  WARNING This file was generated from InternalNAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_QUADS_RESTRAINT_H
#define IMPKERNEL_INTERNAL_INTERNAL_QUADS_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/base/Pointer.h>
#include "../QuadScore.h"
#include "../QuadContainer.h"
#include "../Restraint.h"
#include "ContainerRestraint.h"


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Applies a QuadScore to each Quad in a list.
/** This restraint stores the used particles in a ParticleQuadsTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListQuadContainer is used and the
    {add_, set_, clear_}particle_quad{s} methods can be used.

    \see QuadRestraint
 */
class InternalQuadsRestraint :
    public ContainerRestraint<QuadScore, QuadContainer>
{
public:
  InternalQuadsRestraint(QuadScore *ss,
                              QuadContainer *pc,
                              std::string name="QuadsRestraint %1%"):
      ContainerRestraint<QuadScore, QuadContainer>(ss, pc, name)
      {


  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_QUADS_RESTRAINT_H */

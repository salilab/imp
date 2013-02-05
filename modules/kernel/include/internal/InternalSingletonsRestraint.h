/**
 *  \file CoreSingletonsRestraint.h
 *  \brief Apply a SingletonScore to each Singleton in a list.
 *
 *  WARNING This file was generated from InternalNAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_SINGLETONS_RESTRAINT_H
#define IMPKERNEL_INTERNAL_INTERNAL_SINGLETONS_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/base/Pointer.h>
#include "../SingletonScore.h"
#include "../SingletonContainer.h"
#include "../Restraint.h"
#include "ContainerRestraint.h"


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Applies a SingletonScore to each Singleton in a list.
/** This restraint stores the used particles in a ParticlesTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListSingletonContainer is used and the
    {add_, set_, clear_}particle{s} methods can be used.

    \see SingletonRestraint
 */
class InternalSingletonsRestraint :
    public ContainerRestraint<SingletonScore, SingletonContainer>
{
public:
  InternalSingletonsRestraint(SingletonScore *ss,
                              SingletonContainer *pc,
                              std::string name="SingletonsRestraint %1%"):
      ContainerRestraint<SingletonScore, SingletonContainer>(ss, pc, name)
      {


  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_SINGLETONS_RESTRAINT_H */

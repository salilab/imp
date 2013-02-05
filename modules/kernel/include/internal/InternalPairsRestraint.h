/**
 *  \file CorePairsRestraint.h
 *  \brief Apply a PairScore to each Pair in a list.
 *
 *  WARNING This file was generated from InternalNAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_PAIRS_RESTRAINT_H
#define IMPKERNEL_INTERNAL_INTERNAL_PAIRS_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/base/Pointer.h>
#include "../PairScore.h"
#include "../PairContainer.h"
#include "../Restraint.h"
#include "ContainerRestraint.h"


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Applies a PairScore to each Pair in a list.
/** This restraint stores the used particles in a ParticlePairsTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListPairContainer is used and the
    {add_, set_, clear_}particle_pair{s} methods can be used.

    \see PairRestraint
 */
class InternalPairsRestraint :
    public ContainerRestraint<PairScore, PairContainer>
{
public:
  InternalPairsRestraint(PairScore *ss,
                              PairContainer *pc,
                              std::string name="PairsRestraint %1%"):
      ContainerRestraint<PairScore, PairContainer>(ss, pc, name)
      {


  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_PAIRS_RESTRAINT_H */

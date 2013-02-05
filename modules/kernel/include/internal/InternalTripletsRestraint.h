/**
 *  \file CoreTripletsRestraint.h
 *  \brief Apply a TripletScore to each Triplet in a list.
 *
 *  WARNING This file was generated from InternalNAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_TRIPLETS_RESTRAINT_H
#define IMPKERNEL_INTERNAL_INTERNAL_TRIPLETS_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/base/Pointer.h>
#include "../TripletScore.h"
#include "../TripletContainer.h"
#include "../Restraint.h"
#include "ContainerRestraint.h"


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Applies a TripletScore to each Triplet in a list.
/** This restraint stores the used particles in a ParticleTripletsTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListTripletContainer is used and the
    {add_, set_, clear_}particle_triplet{s} methods can be used.

    \see TripletRestraint
 */
class InternalTripletsRestraint :
    public ContainerRestraint<TripletScore, TripletContainer>
{
public:
  InternalTripletsRestraint(TripletScore *ss,
                              TripletContainer *pc,
                              std::string name="TripletsRestraint %1%"):
      ContainerRestraint<TripletScore, TripletContainer>(ss, pc, name)
      {


  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_TRIPLETS_RESTRAINT_H */

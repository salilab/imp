/**
 *  \file CoreCLASSNAMEsRestraint.h
 *  \brief Apply a CLASSNAMEScore to each CLASSNAME in a list.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_HEADERNAMES_RESTRAINT_H
#define IMPKERNEL_INTERNAL_INTERNAL_HEADERNAMES_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/base/Pointer.h>
#include "../CLASSNAMEScore.h"
#include "../CLASSNAMEContainer.h"
#include "../Restraint.h"
#include "ContainerRestraint.h"


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Applies a CLASSNAMEScore to each CLASSNAME in a list.
/** This restraint stores the used particles in a PLURALVARIABLETYPE.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListCLASSNAMEContainer is used and the
    {add_, set_, clear_}FUNCTIONNAME{s} methods can be used.

    \see CLASSNAMERestraint
 */
class InternalCLASSNAMEsRestraint :
    public ContainerRestraint<CLASSNAMEScore, CLASSNAMEContainer>
{
public:
  InternalCLASSNAMEsRestraint(CLASSNAMEScore *ss,
                              CLASSNAMEContainer *pc,
                              std::string name="CLASSNAMEsRestraint %1%"):
      ContainerRestraint<CLASSNAMEScore, CLASSNAMEContainer>(ss, pc, name)
      {


  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_HEADERNAMES_RESTRAINT_H */

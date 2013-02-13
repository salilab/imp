/**
 *  \file CoreClassnamesRestraint.h
 *  \brief Apply a ClassnameScore to each Classname in a list.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_CLASSNAMES_RESTRAINT_H
#define IMPKERNEL_INTERNAL_INTERNAL_CLASSNAMES_RESTRAINT_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/base/Pointer.h>
#include "../ClassnameScore.h"
#include "../ClassnameContainer.h"
#include "../Restraint.h"
#include "ContainerRestraint.h"


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Applies a ClassnameScore to each Classname in a list.
/** This restraint stores the used particles in a PLURALVARIABLETYPE.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListClassnameContainer is used and the
    {add_, set_, clear_}FUNCTIONNAME{s} methods can be used.

    \see ClassnameRestraint
 */
class InternalClassnamesRestraint :
    public ContainerRestraint<ClassnameScore, ClassnameContainer>
{
public:
  InternalClassnamesRestraint(ClassnameScore *ss,
                              ClassnameContainer *pc,
                              std::string name="ClassnamesRestraint %1%"):
      ContainerRestraint<ClassnameScore, ClassnameContainer>(ss, pc, name)
      {


  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_INTERNAL_CLASSNAMES_RESTRAINT_H */

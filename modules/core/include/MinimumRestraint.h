/**
 *  \file MinimumRestraint.h   \brief Score based on the k minimum restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MINIMUM_RESTRAINT_H
#define IMPCORE_MINIMUM_RESTRAINT_H

#include "core_config.h"
#include <IMP/Restraint.h>
#include <IMP/macros.h>

IMPCORE_BEGIN_NAMESPACE

//! Score based on the minimum scoring members of a set of restraints
/**
 */
class IMPCOREEXPORT MinimumRestraint :  public Restraint
{
  unsigned int k_;
public:
  /** Score based on the num minimum restraints from rs.
   */
  MinimumRestraint(unsigned int num,
                   const Restraints& rs=Restraints(),
                   std::string name="MinimumRestraint %1%");

  IMP_RESTRAINT(MinimumRestraint);

  IMP_LIST(public, Restraint, restraint, Restraint*, Restraints);
  void set_model(Model *m);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MINIMUM_RESTRAINT_H */

/**
 *  \file LowestNRestraintSet.h
 *  \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_LOWEST_NRESTRAINT_SET_H
#define IMPCORE_LOWEST_NRESTRAINT_SET_H

#include "core_exports.h"
#include "internal/core_version_info.h"

#include <IMP/Restraint.h>

#include <string>

IMPCORE_BEGIN_NAMESPACE

//! Return the score computed from the lowest n scoring restraints in the set
/** \ingroup restraint
 */
class IMPCOREEXPORT LowestNRestraintSet : public Restraint
{
  unsigned int n_;
public:
  LowestNRestraintSet(unsigned int n);
  ~LowestNRestraintSet();
  virtual ParticlesList get_interacting_particles() const;


  IMP_RESTRAINT(internal::core_version_info)

  IMP_CONTAINER(Restraint, restraint, RestraintIndex);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_LOWEST_NRESTRAINT_SET_H */

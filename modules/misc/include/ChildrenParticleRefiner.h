/**
 *  \file ChildrenParticleRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_CHILDREN_PARTICLE_REFINER_H
#define IMPMISC_CHILDREN_PARTICLE_REFINER_H

#include "misc_exports.h"
#include "internal/misc_version_info.h"

#include <IMP/ParticleRefiner.h>

IMPMISC_BEGIN_NAMESPACE

//! Return the hierarchy children of a particle.
/** Derivatives are not propagated back to parent or anything.
*/
class IMPMISCEXPORT ChildrenParticleRefiner : public ParticleRefiner
{
public:
  ChildrenParticleRefiner();

  virtual ~ChildrenParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::misc_version_info);
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_CHILDREN_PARTICLE_REFINER_H */

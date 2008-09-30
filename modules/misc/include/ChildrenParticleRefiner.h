/**
 *  \file ChildrenParticleRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPMISC_CHILDREN_PARTICLE_REFINER_H
#define __IMPMISC_CHILDREN_PARTICLE_REFINER_H

#include "IMP/ParticleRefiner.h"
#include "misc_version_info.h"

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

#endif  /* __IMPMISC_CHILDREN_PARTICLE_REFINER_H */

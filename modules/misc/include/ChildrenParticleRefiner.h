/**
 *  \file ChildrenParticleRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_CHILDREN_PARTICLE_REFINER_H
#define __IMP_CHILDREN_PARTICLE_REFINER_H

#include "IMP/ParticleRefiner.h"
#include "misc_version_info.h"

namespace IMP
{

namespace misc
{

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

} // namespace misc

} // namespace IMP

#endif  /* __IMP_CHILDREN_PARTICLE_REFINER_H */

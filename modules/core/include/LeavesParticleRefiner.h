/**
 *  \file LeavesParticleRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_LEAVES_PARTICLE_REFINER_H
#define IMPCORE_LEAVES_PARTICLE_REFINER_H

#include "config.h"
#include "internal/version_info.h"
#include "HierarchyDecorator.h"
#include <IMP/atom/MolecularHierarchyDecorator.h>

#include <IMP/ParticleRefiner.h>

IMPCORE_BEGIN_NAMESPACE

class HierarchyTraits;

//! Return the hierarchy leaves under a particle.
/** \ingroup hierarchy
    \see HierarchyDecorator
    \see MolecularHierarchyDecorator
*/
class IMPCOREEXPORT LeavesParticleRefiner : public ParticleRefiner
{

  HierarchyTraits traits_;
public:
  //! Create a refiner for a particular type of hierarchy
  LeavesParticleRefiner(HierarchyTraits tr);

  virtual ~LeavesParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_LEAVES_PARTICLE_REFINER_H */

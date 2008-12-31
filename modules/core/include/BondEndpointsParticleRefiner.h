/**
 *  \file BondEndpointsParticleRefiner.h
 *  \brief Return the endpoints of a bond.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_ENDPOINTS_PARTICLE_REFINER_H
#define IMPCORE_BOND_ENDPOINTS_PARTICLE_REFINER_H

#include "config.h"
#include "internal/core_version_info.h"

#include <IMP/ParticleRefiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Return the endpoints of a bond.
/**
 \ingroup hierarchy
 */
class IMPCOREEXPORT BondEndpointsParticleRefiner : public ParticleRefiner
{
public:
  //! no arguments
  BondEndpointsParticleRefiner();

  virtual ~BondEndpointsParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::core_version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_ENDPOINTS_PARTICLE_REFINER_H */

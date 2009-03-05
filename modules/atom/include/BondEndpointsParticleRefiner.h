/**
 *  \file BondEndpointsParticleRefiner.h
 *  \brief Return the endpoints of a bond.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_ENDPOINTS_PARTICLE_REFINER_H
#define IMPATOM_BOND_ENDPOINTS_PARTICLE_REFINER_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/ParticleRefiner.h>

IMPATOM_BEGIN_NAMESPACE

//! Return the endpoints of a bond.
/**
 \ingroup bond
 \see BondDecorator
 */
class IMPATOMEXPORT BondEndpointsParticleRefiner : public ParticleRefiner
{
public:
  //! no arguments
  BondEndpointsParticleRefiner();

  virtual ~BondEndpointsParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::version_info);
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_ENDPOINTS_PARTICLE_REFINER_H */

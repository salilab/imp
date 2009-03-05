/**
 *  \file core/BondEndpointsParticleRefiner.h
 *  \brief Return the endpoints of a bond.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_ENDPOINTS_PARTICLE_REFINER_H
#define IMPCORE_BOND_ENDPOINTS_PARTICLE_REFINER_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/ParticleRefiner.h>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Return the endpoints of a bond.
/** \deprecated Use atom::BondEndpointsParticleRefiner
 \ingroup bond
 \see BondDecorator
 */
class IMPCOREEXPORT BondEndpointsParticleRefiner : public ParticleRefiner
{
public:
  //! no arguments
  BondEndpointsParticleRefiner();

  virtual ~BondEndpointsParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::version_info);
};

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_ENDPOINTS_PARTICLE_REFINER_H */

/**
 *  \file BondCoverParticleRefiner.h
 *  \brief Cover a bond with a constant volume set of spheres
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_BOND_COVER_PARTICLE_REFINER_H
#define IMPMISC_BOND_COVER_PARTICLE_REFINER_H

#include "misc_exports.h"
#include "internal/misc_version_info.h"

#include <IMP/ParticleRefiner.h>

IMPMISC_BEGIN_NAMESPACE

//! Cover a bond with a constant volume set of spheres.
/** Perhaps I want to add various custom bond types so that
    this will only expand some custom bonds. Currently any
    particle which is an instance of BondDecorator is expanded.
*/
class IMPMISCEXPORT BondCoverParticleRefiner : public ParticleRefiner
{
  FloatKey rk_;
  FloatKey vk_;
 public:
  /** Cover the bond with spheres. Their volumes will add up to the number
      given in the vk field of the bond and their radii will be set using rk */
  BondCoverParticleRefiner(FloatKey rk,
                           FloatKey vk);

  virtual ~BondCoverParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::misc_version_info);
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_BOND_COVER_PARTICLE_REFINER_H */

/**
 *  \file BondCoverParticleRefiner.h
 *  \brief Cover a bond with a constant volume set of spheres
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BOND_COVER_PARTICLE_REFINER_H
#define __IMP_BOND_COVER_PARTICLE_REFINER_H

#include "../internal/kernel_version_info.h"
#include "../ParticleRefiner.h"

namespace IMP
{

//! Cover a bond with a constant volume set of spheres.
/** Perhaps I want to add various custom bond types so that
    this will only expand some custom bonds. Currently any
    particle which is an instance of BondDecorator is expanded.
*/
class IMPDLLEXPORT BondCoverParticleRefiner : public ParticleRefiner
{
  FloatKey rk_;
  FloatKey vk_;
public:
  BondCoverParticleRefiner(FloatKey rk,
                           FloatKey vk);

  virtual ~BondCoverParticleRefiner() {}

  IMP_PARTICLE_REFINER(internal::kernel_version_info);
};

} // namespace IMP

#endif  /* __IMP_BOND_COVER_PARTICLE_REFINER_H */

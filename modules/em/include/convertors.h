/**
 *  \file convertors.h
 *  \brief Convertors of density values
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_CONVERTORS_H
#define IMPEM_CONVERTORS_H

#include "config.h"
#include <IMP/base_types.h>
#include <IMP/macros.h>
#include "internal/version_info.h"
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include <IMP/em/IMPParticlesAccessPoint.h>
#include <IMP/Particle.h>
#include "IMP/core/XYZDecorator.h"
#include "IMP/algebra/Vector3D.h"

IMPEM_BEGIN_NAMESPACE

//! Return the key used to store the density attribute
IMPEMEXPORT FloatKey get_density_key();

//! Converts a density grid to a set of paritlces
/**
Each such particle will be have xyz attributes and a density_val attribute of
type Float.
/param[in] dmap the density map
/param[in] threshold only voxels with density above the given threshold will
           be converted to particles
/param[in] the new density based particles will be converted to particles.
 */
inline void density2particles(DensityMap &dmap, Float threshold,
                              Particles &ps, Model *m) {
  Float x,y,z;
  for (long i=0;i<dmap.get_number_of_voxels();i++) {
    x = dmap.voxel2loc(i,0);
    y = dmap.voxel2loc(i,1);
    z = dmap.voxel2loc(i,2);
    if (dmap.get_value(x,y,z) > threshold) {
      Particle * p = new Particle(m);
      IMP::core::XYZDecorator::create(p,IMP::algebra::Vector3D(x,y,z));
      p->add_attribute(get_density_key(),
                       dmap.get_value(x,y,z),false);
      ps.push_back(p);
    }
  }
}


//! Resample a set of particles into a density grid
/**
Each such particle should be have xyz radius and weight attributes
/param[in] ps         the particles to sample
/param[in] rad_key    the radius attribute key of the particles
/param[in] weight_key the weight attribute key of the particles
/param[in] ps  the particles to sample
/param[in] resolution the resolution of the new sampled map
/param[in] apix the voxel size of the sampled map
/return the sampled density grid
 */
inline SampledDensityMap * particles2density(Particles &ps,
               const FloatKey &rad_key, const FloatKey &weight_key,
               Float resolution, Float apix) {
  IMPParticlesAccessPoint access_p(ps,rad_key,weight_key);
  SampledDensityMap *dmap = new SampledDensityMap(access_p, resolution,
                                                  apix);
  return dmap;
}

IMPEM_END_NAMESPACE
#endif /* IMPEM_CONVERTORS_H */

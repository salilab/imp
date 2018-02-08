/**
 *  \file IMP/em/converters.h
 *  \brief Converters of density values
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_CONVERTERS_H
#define IMPEM_CONVERTERS_H

#include <IMP/em/em_config.h>
#include <IMP/base_types.h>
#include <IMP/macros.h>
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include "SurfaceShellDensityMap.h"
#include <IMP/Particle.h>
#include "IMP/core/XYZ.h"
#include "IMP/algebra/Vector3D.h"

IMPEM_BEGIN_NAMESPACE

//! Convert a density grid to a set of particles
/** Each such particle will have xyz attributes and a density_val attribute
    of type Float.
    \param[in] dmap the density map
    \param[in] threshold only voxels with density above the given threshold will
               be converted to particles
    \param[in] m model to store the new particles
    \param[in] step sample every X steps in each direction
    \return particles corresponding to all voxels above the threshold
 */
IMPEMEXPORT Particles density2particles(DensityMap *dmap,
                                        Float threshold,
                                        Model *m, int step = 1);

//! Convert a density grid to a set of vectors
/** \param[in] dmap the density map
    \param[in] threshold only voxels with density above the given threshold will
               be converted to vectors
    \return Vector3Ds corresponding to the positions of all voxels
            above the threshold
 */
IMPEMEXPORT algebra::Vector3Ds density2vectors(DensityMap *dmap,
                                               Float threshold);

//! Resample a set of particles into a density grid
/**
Each such particle should have xyz, radius and weight attributes
\param[in] ps         the particles to sample
\param[in] resolution the resolution of the new sampled map
\param[in] apix the voxel size of the sampled map
\param[in] sig_cutoff sigma cutoff used in sampling
\param[in] weight_key the weight attribute key of the particles
\return the sampled density grid
\see SampledDensityMap
 */
IMPEMEXPORT SampledDensityMap *particles2density(
    const ParticlesTemp &ps, Float resolution, Float apix,
    int sig_cutoff = 3,
    const FloatKey &weight_key = IMP::atom::Mass::get_mass_key());

//! Resample a set of particles into a binarized density map
//! 1 for voxels containing particles and 0 otherwise
/**
Each such particle should have xyz radius and weight attributes
\param[in] ps         the particles to sample
\param[in] resolution the resolution of the new sampled map
\param[in] apix the voxel size of the sampled map
\param[in] sig_cutoff sigma cutoff used in sampling
\param[in] weight_key the weight attribute key of the particles
\return the sampled density grid
\see SampledDensityMap
 */
inline IMPEMEXPORT SampledDensityMap *particles2binarized_density(
    const ParticlesTemp &ps, Float resolution, Float apix,
    int sig_cutoff = 3,
    const FloatKey &weight_key = IMP::atom::Mass::get_mass_key()) {

  Pointer<SampledDensityMap> dmap(new SampledDensityMap(
      ps, resolution, apix, weight_key, sig_cutoff, BINARIZED_SPHERE));
  return dmap.release();
}

//! Resample a set of particles into a density grid
/**
Each such particle should have xyz radius and weight attributes
\param[in] ps         the particles to sample
\param[in] apix the voxel size of the surface map
\param[in] weight_key the weight attribute key of the particles
\return the surface grid
\see SampledDensityMap
 */
IMPEMEXPORT SurfaceShellDensityMap *particles2surface(
    const ParticlesTemp &ps, Float apix,
    const FloatKey &weight_key = IMP::atom::Mass::get_mass_key());

IMPEMEXPORT Float
    calculate_intersection_score(const SurfaceShellDensityMap *d1,
                                 const SurfaceShellDensityMap *d2);

IMPEM_END_NAMESPACE
#endif /* IMPEM_CONVERTERS_H */

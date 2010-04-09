/**
 *  \file converters.h
 *  \brief Converters of density values
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_CONVERTERS_H
#define IMPEM_CONVERTERS_H

#include "em_config.h"
#include <IMP/base_types.h>
#include <IMP/macros.h>
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include <IMP/Particle.h>
#include "IMP/core/XYZ.h"
#include "IMP/algebra/Vector3D.h"

IMPEM_BEGIN_NAMESPACE

//! Converts a density grid to a set of paritlces
/**
Each such particle will be have xyz attributes and a density_val attribute of
type Float.
\param[in] dmap the density map
\param[in] threshold only voxels with density above the given threshold will
           be converted to particles
\param[in] m model to store the new particles
\param[in] step sample every X steps in each direction
\return particles corresponding to all voxels above the threshold
 */
IMPEMEXPORT Particles density2particles(DensityMap &dmap, Float threshold,
                                        Model *m,int step=1);


//! Converts a density grid to a set of paritlces
/**
Each such particle will be have xyz attributes and a density_val attribute of
type Float.
\param[in] dmap the density map
\param[in] threshold only voxels with density above the given threshold will
           be converted to particles
\return a set of vector3Ds corresponding to the positions of all voxels
        above the threshold
 */
IMPEMEXPORT std::vector<algebra::VectorD<3> >
    density2vectors(DensityMap &dmap, Float threshold);

//! Resample a set of particles into a density grid
/**
Each such particle should be have xyz radius and weight attributes
\param[in] ps         the particles to sample
\param[in] resolution the resolution of the new sampled map
\param[in] apix the voxel size of the sampled map
\param[in] sig_cutoff sigma cutoff used in sampling
\param[in] rad_key   the radius attribute key of the particles
\param[in] weight_key the weight attribute key of the particles
\return the sampled density grid
\relatesalso SampledDensityMap
 */
IMPEMEXPORT SampledDensityMap * particles2density(
   const Particles &ps,
   Float resolution, Float apix,
   int sig_cutoff=3,
   const FloatKey &rad_key=IMP::core::XYZR::get_default_radius_key(),
   const FloatKey &weight_key=IMP::atom::Mass::get_mass_key());
IMPEM_END_NAMESPACE
#endif /* IMPEM_CONVERTERS_H */

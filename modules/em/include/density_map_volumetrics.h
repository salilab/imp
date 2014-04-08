/**
    *  \file IMP/em/density_map_volumetrics.h
    *  \brief Classes and functions to handle
    *  volumetric properties in Density maps
    *
    */

#ifndef IMPEM_DENSITY_MAP_VOLUMETRICS_H
#define IMPEM_DENSITY_MAP_VOLUMETRICS_H

#include <IMP/em/em_config.h>
#include <IMP/em/DensityMap.h>
#include <IMP/base_types.h>  // Float
#include <IMP/em/em_config.h>
#include <IMP/atom/estimates.h>

IMPEM_BEGIN_NAMESPACE
//! Compute an approximate volume
/** Compute an approximate volume for the set of voxels with intensity
    under a given threshold
\param[in] m a density map
\param[in] threshold consider volume of only voxels above this threshold
\return a volume for the set of voxels with intensity
under the provided threshold
    */
IMPEMEXPORT Float get_volume_at_threshold(DensityMap* m, Float threshold);

//! Compute an approximate molecular mass
/** Compute an approximate molecular mass for the set of voxels with intensity
    under a given threshold
\param[in] m a density map
\param[in] threshold only voxels above this threshold will be considered
\param[in] ref the protein density reference to use in the computation.
The default protein density for this computation is HARPAZ
\return an approximate molecular mass for the set of voxels with intensity
under the provided threshold (mass in Da)
    */
IMPEMEXPORT Float
    get_molecular_mass_at_threshold(DensityMap* m, Float threshold,
                                    atom::ProteinDensityReference ref =
                                        atom::HARPAZ);

//! Computes the threshold consider in an EM map to get a desired volume
/** Computes the threshold consider in an EM map to get a desired volume
    * (i.e, the set of voxels with intensity greater than the threshold
    * occupies that volume)
\param[in] m a density map
\param[in] desired_volume (in A^3)
    */
IMPEMEXPORT Float
    get_threshold_for_approximate_volume(DensityMap* m, Float desired_volume);

//! Computes the threshold to consider in an EM map to get a desired mass
/** Computes the threshold to consider in an EM map to get a desired mass
    * (only voxels with intensity greater than the threshold are considered)
\param[in] m a density map
\param[in] desired_mass (in Da)
\param[in] ref the protein density reference to use in the computation.
The default protein density for this computation is HARPAZ
    */
IMPEMEXPORT Float
    get_threshold_for_approximate_mass(DensityMap* m, Float desired_mass,
                                       atom::ProteinDensityReference ref =
                                           atom::HARPAZ);

IMPEM_END_NAMESPACE

#endif /* IMPEM_DENSITY_MAP_VOLUMETRICS_H */

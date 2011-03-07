/**
 *  \file DensityMapVolumetrics.h
 *  \brief Classes and functions to handle volumetric properties in Density maps
 *
 */

#ifndef IMPEM_DENSITY_MAP_VOLUMETRICS_H
#define IMPEM_DENSITY_MAP_VOLUMETRICS_H

#include <IMP/em/DensityMap.h>
#include <IMP/base_types.h>         // Float
#include <IMP/em/em_config.h>

IMPEM_BEGIN_NAMESPACE

/*! A list of several protein density references that have been proposed in
 * the literature.
 *  These constants are meant to be used with the set_used_protein_density()
 *  function to set the  protein density to one of these reference values
 *  - HARPAZ and al (1994)           Computed value  1.37 g/cm3 ~ 0.82644 Da/A3
 *  - ANDERSSON and Hovmšller (1998) Computed value  1.22 g/cm3 ~ 0.7347 Da/A3
 *  - TSAI et al. (1999)             Computed value  1.40 g/cm3 ~ 0.84309 Da/A3
 *  - QUILLIN and Matthews (2000)    Computed value  1.43 g/cm3 ~ 0.86116 Da/A3
 *  - SQUIRE and Himmel (1979),
 *    Gekko and Noguchi (1979) Experimental value  1.37 g/cm3 ~ 0.82503 Da/A3
 *
 * Unless otherly specified, the default value is
 * 0.826446 (Harpaz et al 1994).
 */
enum ProteinDensityReference {
  HARPAZ,
  ANDERSSON,
  TSAI,
  QUILLIN,
  SQUIRE
 } ;


/*! returns the protein density value used for internal computations
 * (in Da/Angstrom^3)
 */
IMPEMEXPORT double get_used_protein_density();


//! sets the protein density value used for internal computations
/*! sets the protein density value used for internal computations
 * \param[in] density_value : the protein density value that is used in internal
 * computations (in Da/Angstrom^3)
 */
IMPEMEXPORT double set_used_protein_density(double density_value);


//! sets the protein density value used for internal computations
/*! sets the protein density value used for internal computations
 * \param[in] density_reference : a reference protein density value that
 * we wish to use in our computations (in Da/Angstrom^3).
 *
 * See ProteinDensityReference for authorized values.
 */
IMPEMEXPORT double set_used_protein_density(
      ProteinDensityReference density_reference);


//! returns the current protein density value
/*! returns the protein density value as it has been defined
 *  in a reference paper
 * \param[in] density_reference : the name of a reference
 *   (See ProteinDensityReference for authorized values)
 *
 * NB : densities are expressed in Da/Angstrom^3
 */
IMPEMEXPORT double get_reference_protein_density(
      ProteinDensityReference density_reference);


//! Compute an approximate volume
/** Compute an approximate volume for the set of voxels with intensity
 * under a given threshold
\param[in] m a density map
\param[in] threshold, consider volume of only voxels above this threshold
\return a volume for the set of voxels with intensity
under the provided threshold
 */
IMPEMEXPORT Float get_volume_at_threshold(DensityMap* m, Float threshold);

//! Compute an approximate molecular mass
/** Compute an approximate molecular mass for the set of voxels with intensity
 * under a given threshold
\param[in] m a density map
\param[in] threshold, only voxels above this threshold will be considered
\return an approximate molecular mass for the set of voxels with intensity
under the provided threshold (mass in Da)
\note By default, the method assumes 0.826446 dalton per cubic A
(Harpaz et al 1994).
this constant can be modified through the set_used_protein_density function.
 */
IMPEMEXPORT Float get_molecular_mass_at_threshold(
      DensityMap* m,
      Float threshold);

//! Computes the threshold consider in an EM map to get a desired volume
/** Computes the threshold consider in an EM map to get a desired volume
 * (i.e, the set of voxels with intensity greater than the threshold
 * occupies that volume)
\param[in] m a density map
\param[in] desired_volume (in A^3)
 */
IMPEMEXPORT Float get_threshold_for_approximate_volume(
      DensityMap* m,
      Float desired_volume);


//! Computes the threshold to consider in an EM map to get a desired mass
/** Computes the threshold to consider in an EM map to get a desired mass
 * (only voxels with intensity greater than the threshold are considered)
\param[in] m a density map
\param[in] desired_mass (in Da)
\note By default, the method assumes 0.826446 Da per cubic A
 (Harpaz et al 1994).
this constant can be modified through the set_used_protein_density function.
 */
IMPEMEXPORT Float get_threshold_for_approximate_mass(
      DensityMap* m,
      Float desired_mass);

IMPEM_END_NAMESPACE

#endif  /* IMPEM_DENSITY_MAP_VOLUMETRICS_H */

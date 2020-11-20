/**
    *  \file IMP/atom/estimates.h
    *  \brief Estimates of various physical quantities.
    *
    *  Copyright 2007-2020 IMP Inventors. All rights reserved.
    */

#ifndef IMPATOM_ESTIMATES_H
#define IMPATOM_ESTIMATES_H

#include <IMP/atom/atom_config.h>
#include "Residue.h"
#include <IMP/base_types.h>

IMPATOM_BEGIN_NAMESPACE
/** @name Estimator Functions

    These functions allow you to estimate physical quantities
    relating to biomolecules.
*/
//!@{

/*! Several protein density value references that have been proposed
 *  in the literature.
 *  - ALBER et al. (structure 2005)  Estimated value 0.625 (1/1.60) Da/A3
 *  - HARPAZ et al. (1994)           Computed value  0.826446=1/1.21 Da/A3
 *  - ANDERSSON and Hovmuller (1998) Computed value  1.22 g/cm3 ~ 0.7347 Da/A3
 *  - TSAI et al. (1999)             Computed value  1.40 g/cm3 ~ 0.84309 Da/A3
 *  - QUILLIN and Matthews (2000)    Computed value  1.43 g/cm3 ~ 0.86116 Da/A3
 *  - SQUIRE and Himmel (1979),
 *    Gekko and Noguchi (1979) Experimental value  1.37 g/cm3 ~ 0.82503 Da/A3
 */
enum ProteinDensityReference {
  ALBER,
  HARPAZ,
  ANDERSSON,
  TSAI,
  QUILLIN,
  SQUIRE
};

//! returns the protein density value (in Da/A^3)
//! associated with a given reference
IMPATOMEXPORT double get_protein_density_from_reference
( ProteinDensityReference densityReference );

//! Estimate the volume of a protein from its mass
/**
 * \param[in] m   the mass for which we want to output the corresponding volume
 * \param[in] ref the protein density reference used in the computation.
 * As a default ref is the estimate published in Alber et. al, Structure 2005.
*/
IMPATOMEXPORT double get_volume_from_mass
( double m,
  ProteinDensityReference ref = ALBER);

//! Estimate the mass of a protein from its volume
/**
 * \param[in] v   the volume for which we want to output the corresponding mass
 * \param[in] ref the protein density reference used in the computation.
 * As a default ref is the estimate published in Alber et. al, Structure 2005.
*/
IMPATOMEXPORT double get_mass_from_volume
( double v,
  ProteinDensityReference ref = ALBER );

//! Estimate the mass of a protein from the number of amino acids
/** We use an estimate of 110 Daltons per residue, following Chimera.

    The mass is in Daltons.
 */
IMPATOMEXPORT double get_mass_from_number_of_residues
( unsigned int num_aa );

//! Return an estimate for the volume of a given residue
/** The volume estimates are taken from
 Pontius J, Richelle J, Wodak SJ.,
 \external{https://www.ncbi.nlm.nih.gov/pubmed/8950272,
 Deviations from standard atomic volumes as a quality measure for
 protein crystal structures}, J Mol Biol. 1996 Nov 22;264(1):121-36.


 \throw ValueException if a non-standard residue type is passed
 */
IMPATOMEXPORT double get_volume_from_residue_type
( ResidueType rt );

//!@}

/** Compute the concentration in molarity from the passed values*/
inline double get_molarity
( double n,
  double volume ) {
  double v = volume;
  // n*10^27/(v *6.02e23)
  return n * 1e4 / (v * 6.02);
}

/** Compute the concentration in molarity from the passed values*/
inline double get_kd
( double na,
  double nb,
  double nab,
  double volume ) {
  return get_molarity(na, volume) * get_molarity(nb, volume) /
         get_molarity(nab, volume);
}

/** Return the predicted diffusion coefficient in Angstrom squared per
    femtosecond at the specified radius and temperature.

    @param particle radius in angstroms
    @param temp  temperature in Kelvin
                 (negative = default IMP temperature of 297.15K)

    See
    \external{http://en.wikipedia.org/wiki/Einstein_relation_(kinetic_theory),
    wikipedia} for a reference and
    \external{http://en.wikipedia.org/wiki/Viscosity,Wikipedia on Viscosity}
    for the values of the viscosity of water used.*/
IMPATOMEXPORT double get_einstein_diffusion_coefficient
( double r,
  double temp= -1);

/** Return the predicted diffusion coefficient in radians squared per
    femtosecond at the specified radius and temperature.

    @param particle radius in angstroms
    @param temp  temperature in Kelvin
                 (negative = default IMP temperature of 297.15K)

*/
IMPATOMEXPORT double get_einstein_rotational_diffusion_coefficient
( double r,
  double temp= -1 );

/** Return the standard deviation for the Brownian step in Angstroms given the
    diffusion coefficient D in A^2/fs and the time step t in fs.*/
IMPATOMEXPORT double get_diffusion_length
( double D, double t );

/** Return the scale for diffusion in Angstroms given the specified force,
    the diffusion coefficient D and the time step t.

    @param D     diffusion coefficient in Angstrom^2/femtosecond
    @param force applied force in kcal/mol/A
    @param t     time step in femtoseconds
    @param temp  temperature in Kelvin
                 (negative = default IMP temperature of 297.15K)
*/
IMPATOMEXPORT double get_diffusion_length
( double D,
  double force,
  double t,
  double temp = -1);

/** Get the standard deviation of the diffusion angle change in radians given
    the rigid body diffusion coefficient in A^2/fs and the time step dtfs in fs.*/
IMPATOMEXPORT double get_diffusion_angle
( double D,
  double dtfs );

/** Estimate the diffusion coefficient of a particle in A^2/fs from a list of
    displacements each taken after the given time step dt. Removes any non-random
    motion component (under the simplifying assumption it is constant over time)

    @param displacements displacement vectors in Angstroms
    @param dt time step in femtoseconds

    @note The units of displacements and dt params could be actually
    arbitrary.  The units of thE returned value will simply change to
    e.g.  cm^2/sec if displacements and dt are specified in cm and
    seconds, resp.

*/
IMPATOMEXPORT double get_diffusion_coefficient
( const algebra::Vector3Ds &displacements, double dt );

/** Estimate the diffusion coefficient of a particle in A^2/fs from a list of
    displacements each taken after the given time step dt. Removes any non-random
    motion component (under the simplifying assumption it is constant over time)

    @param displacements displacement vectors in Angstroms
    @param dts corresponding time steps in femtoseconds, expected to sum to more than zero

    @note The units of displacements and dt params could be actually
    arbitrary.  The units of thE returned value will simply change to
    e.g.  cm^2/sec if displacements and dts are specified in cm and
    seconds, resp.
*/
IMPATOMEXPORT double get_diffusion_coefficient
( const algebra::Vector3Ds &displacements,
  const Floats &dts);

/** Estimate the rotational diffusion coefficient of a particle in Rad^2/fs
    from a list of rotational orientations taken at consecutive time steps dt
    (in fs). It is assumed that the mean angular rotation is zero (as it is otherwise
    complicated to compute from a list of orientations - may require parameter
    estimation of the folded-normal distribution)
*/
IMPATOMEXPORT double get_rotational_diffusion_coefficient
( const algebra::Rotation3Ds &orientations,
  double dt );

/**\name Energy conversions

   Convert energy from kcal/mol to femtojoules
   @{
*/
IMPATOMEXPORT double get_energy_in_femto_joules
( double energy_in_kcal_per_mol );

//! Convert force from kcal/mol/A to femtonewtons
IMPATOMEXPORT double get_force_in_femto_newtons
( double force_in_kcal_per_mol_per_angstrom );

//! Convert spring constant from kcal/mol/A^2 to femtonewton/A
IMPATOMEXPORT double get_spring_constant_in_femto_newtons_per_angstrom
( double k_in_kcal_per_mol_per_angstrom_square );



IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ESTIMATES_H */

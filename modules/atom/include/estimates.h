/**
    *  \file estimates.h
    *  \brief Estimates of various physical quantities.
    *
    *  Copyright 2007-2011 IMP Inventors. All rights reserved.
    */

#ifndef IMPATOM_ESTIMATES_H
#define IMPATOM_ESTIMATES_H

#include "atom_config.h"
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
 *  - ANDERSSON and Hovmšller (1998) Computed value  1.22 g/cm3 ~ 0.7347 Da/A3
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
IMPATOMEXPORT double get_protein_density_from_reference(
                ProteinDensityReference densityReference);

//! Estimate the volume of a protein from its mass
/**
 * \param[in] m   the mass for which we want to output the corresponding volume
 * \param[in] ref the protein density reference used in the computation.
 * As a default ref is the estimate published in Alber et. al, Structure 2005.
*/
IMPATOMEXPORT double get_volume_from_mass(
                double m,
                ProteinDensityReference ref=ALBER);

//! Estimate the mass of a protein from its volume
/**
 * \param[in] v   the volume for which we want to output the corresponding mass
 * \param[in] ref the protein density reference used in the computation.
 * As a default ref is the estimate published in Alber et. al, Structure 2005.
*/
IMPATOMEXPORT double get_mass_from_volume(
                double v,
                ProteinDensityReference ref=ALBER);

//! Estimate the mass of a protein from the number of amino acids
/** We use an estimate of 110 Daltons per residue, following Chimera.

    The mass is in Daltons.
 */
IMPATOMEXPORT double get_mass_from_number_of_residues(unsigned int num_aa);


//! Return an estimate for the volume of a given residue
/** The volume estimates are taken from
 Pontius J, Richelle J, Wodak SJ.,
 \external{http://www.ncbi.nlm.nih.gov/pubmed/8950272,
 Deviations from standard atomic volumes as a quality measure for
 protein crystal structures}, J Mol Biol. 1996 Nov 22;264(1):121-36.


 \throw ValueException if a non-standard residue type is passed
 */
IMPATOMEXPORT double get_volume_from_residue_type(ResidueType rt);

//!@}


/** Compute the concentration in molarity from the passed values*/
inline double get_molarity(double n, const algebra::BoundingBox3D &bb) {
  double v= algebra::get_volume(bb); // cubic angstrom
  // n*10^27/(v *6.02e23)
  return n*1e4/(v*6.02);
}

/** Compute the concentration in molarity from the passed values*/
inline double get_kd(double na, double nb, double nab,
              const algebra::BoundingBox3D &bb) {
  return get_molarity(na, bb)*get_molarity(nb, bb)/get_molarity(nab, bb);
}


/** Return the prediction diffusion coefficient in Angstrom squared per
    femtosecond given a radius in angstrom.
    See \external{http://en.wikipedia.org/wiki/Einstein_relation_(kinetic_theory)}
    for a reference.*/
IMPATOMEXPORT double get_einstein_diffusion_coefficient(double r);


/** Return the prediction diffusion coefficient in radians squared per
    femtosecond given a radius in angstrom.*/
IMPATOMEXPORT double get_einstein_rotational_diffusion_coefficient(double r);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ESTIMATES_H */

/**
 *  \file estimates.h
 *  \brief Estimates of various physical quantities.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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

//! Estimate the volume of a protein from its mass
/** We use the estimate published in Alber et. al, Structure 2005.
*/
IMPATOMEXPORT double get_volume_from_mass(double v);


//! Estimate the mass of a protein from the number of amino acids
/** We use an estimate of 110 Daltons per residue, following Chimera.

    The mass is in Daltons.
 */
IMPATOMEXPORT double get_mass_from_number_of_residues(unsigned int num_aa);


//! Return an estimate for the volume of a given residue
/** The volume estimates are taken from
 Pontius J, Richelle J, Wodak SJ.,
 \external{www.ncbi.nlm.nih.gov/pubmed/8950272,
 Deviations from standard atomic volumes as a quality measure for
 protein crystal structures}, J Mol Biol. 1996 Nov 22;264(1):121-36.

 */
IMPATOMEXPORT double get_volume_from_residue_type(ResidueType rt);

//!@}

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ESTIMATES_H */

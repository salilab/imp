/**
 *  \file estimates.h
 *  \brief Estimates of various physical quantities.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_ESTIMATES_H
#define IMPATOM_ESTIMATES_H

#include "config.h"
#include <IMP/base_types.h>

IMPATOM_BEGIN_NAMESPACE
/** @name Estimator Functions

    These functions allow you to estimate physical quantities
    relating to biomolecules.
*/
//!@{

//! Estimate the volume of a protein from its mass
/** The estimate uses a formula published in

    <b>Average protein density is a molecular-weight-dependent function.</b>

    Fischer H, Polikarpov I, Craievich AF.

    The mass density of proteins is a relevant basic biophysical
    quantity. It is also a useful input parameter, for example, for
    three-dimensional structure determination by protein
    crystallography and studies of protein oligomers in solution by
    analytic ultracentrifugation. We have performed a critical
    analysis of published, theoretical, and experimental
    investigations about this issue and concluded that the average
    density of proteins is not a constant as often assumed. For
    proteins with a molecular weight below 20 kDa, the average density
    exhibits a positive deviation that increases for decreasing
    molecular weight. A simple molecular-weight-depending function is
    proposed that provides a more accurate estimate of the average
    protein density.

    \note I don't like the abbreviations in the name, but it is too
    long without them.
*/
IMPATOMEXPORT double volume_in_A3_from_mass_in_kDa(double v);


//! Estimate the mass of a protein from the numnber of amino acids
/** This function currently uses the numbers from
    \external{http://en.wikipedia.org/wiki/Protein,wikipedia} that yeast
    proteins average 53kDa in mass and 466 residues in length.
 */
IMPATOMEXPORT double mass_in_kDa_from_number_of_residues(unsigned int num_aa);


//!@}

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ESTIMATES_H */

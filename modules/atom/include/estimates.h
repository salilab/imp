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
/** We use the estimate published in Alber et. al, Structure 2005.
*/
IMPATOMEXPORT double volume_from_mass(double v);


//! Estimate the mass of a protein from the number of amino acids
/** We use an estimate of 110 Daltons per residue, following Chimera.

    The mass is in Daltons.
 */
IMPATOMEXPORT double mass_from_number_of_residues(unsigned int num_aa);

//!@}

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ESTIMATES_H */

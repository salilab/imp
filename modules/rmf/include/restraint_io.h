/**
 *  \file IMP/rmf/restraint_io.h
 *  \brief Manage read/write of Restraints from/to RMF files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_RESTRAINT_IO_H
#define IMPRMF_RESTRAINT_IO_H

#include <IMP/rmf/rmf_config.h>
#include "link_macros.h"
#include <IMP/object_macros.h>
#include <IMP/Restraint.h>

IMPRMF_BEGIN_NAMESPACE

/** \name Restraint I/O
    Restraints are written as RMF::FEATURE nodes with subnodes for
    the decomposed restraints
    (Restraint::create_current_decomposition()).
    The Restraints::get_last_score() value is what is saved to the file,
    so make sure that the restraints have been evaluated before saving
    a frame.

    Any particles returned by Restraint::get_input_particles() that are
    also linked in the RMF file will be included in the RMF file as
    inputs for the Restraint. This allows external software like e.g.,
    Chimera to associate these restraints with a certain set of particles.
    @{
*/
IMP_DECLARE_LINKERS(Restraint, restraint, restraints, Restraint *,
                    Restraints,
                    (RMF::FileConstHandle fh, Model *m), );

//! Add a list of restraints to the file as (static) bonds.
/** There restraints must decompose into pairwise restraints.
    Their scores will not be recorded in the file. */
IMPRMFEXPORT void add_restraints_as_bonds(RMF::FileHandle fh,
                                          const Restraints &rs);

//! Limit the number of restraint terms.
/** Certain restraints are made from a really large number of terms.
    Tracking and displaying all those
    terms can be very time consuming. If the number of terms is larger
    than the maximum, the terms are not displayed. By default this is 100. */
IMPRMFEXPORT void set_maximum_number_of_terms(RMF::FileHandle fh,
                                              unsigned int num);
/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_RESTRAINT_IO_H */

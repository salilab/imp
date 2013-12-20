/**
 *  \file IMP/rmf/restraint_io.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_RESTRAINT_IO_H
#define IMPRMF_RESTRAINT_IO_H

#include <IMP/rmf/rmf_config.h>
#include "link_macros.h"
#include <IMP/base/object_macros.h>
#include <IMP/kernel/Restraint.h>

IMPRMF_BEGIN_NAMESPACE

/** \name kernel::Restraint I/O
    kernel::Restraints are written as am RMF::FEATURE node with subnodes for
    the decomposed restraints
    (kernel::Restraint::create_current_decompositon()).
    The kernel::Restraints::get_last_score() value is what is saved to the file,
    so make sure that the restraints have been evaluated before saving
    a frame.

    Any particles returnd by kernel::Restraint::get_input_particles() that are
    also linked in the RMF file will be included in the RMF file as
    inputs for the kernel::Restraint. This allows external software like e.g.,
    Chimera to associate these restrains with a certain set of particles.
    @{
*/
IMP_DECLARE_LINKERS(Restraint, restraint, restraints, kernel::Restraint *,
                    kernel::Restraints,
                    (RMF::FileConstHandle fh, kernel::Model *m), );

/** Add a list of restraints to the file as (static) bonds. There restraints
 * must decompose into pairwise restraints. Their scores will not be recorded in
 * the file. */
IMPRMFEXPORT void add_restraints_as_bonds(RMF::FileHandle fh,
                                          const kernel::Restraints &rs);

/** Certain restraint are made from a really large number of terms.
    Tracking and displaying all those
    terms can be very time consuming. If the number of terms is larger
    than the maximum, the terms are not displayed. By default this is 100.*/
IMPRMFEXPORT void set_maximum_number_of_terms(RMF::FileHandle fh,
                                              unsigned int num);
/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_RESTRAINT_IO_H */

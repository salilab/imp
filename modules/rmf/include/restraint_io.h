/**
 *  \file IMP/rmf/restraint_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_RESTRAINT_IO_H
#define IMPRMF_RESTRAINT_IO_H

#include "rmf_config.h"
#include <RMF/NodeHandle.h>
#include <IMP/Restraint.h>

IMPRMF_BEGIN_NAMESPACE
/** \name Restraint I/O
    Restraint I/O is quite limited as it is not practical to write enough
    information to the file to recreate actual IMP::Restraint objects
    at the moment. Instead, all that is written is the name, the particles
    used and the score. Restraints that can be decomposed are then
    decomposed and their the decomposed restraints are written as children.
    When the decomposition changes from frame to frame (eg restraints on
    IMP::container::ClosePairContainer containers), the list of particles
    will be empty for frames where that bit of the decomposed restraint
    is not found.
    @{
*/
/** Add a restraint to the file.*/
IMPRMFEXPORT void add_restraint(RMF::RootHandle parent, Restraint *r);
/** Add the current score of the restraint to the given frame.*/
IMPRMFEXPORT void save_frame(RMF::RootHandle parent, int frame, Restraint *r);

/** Return the list of particles for a restraint node. This is not
    such a nice interface, but I can't think of a better one at the
    moment.*/
IMPRMFEXPORT ParticlesTemp get_restraint_particles(RMF::NodeHandle f,
                                                    int frame);

/** Return the restraint score, if available (or -inf if it is not).
    This is not
    such a nice interface, but I can't think of a better one at the
    moment.*/
IMPRMFEXPORT double get_restraint_score(RMF::NodeHandle f,
                                         int frame);

/** @} */
IMPRMF_END_NAMESPACE

#endif /* IMPRMF_RESTRAINT_IO_H */

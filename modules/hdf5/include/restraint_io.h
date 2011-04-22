/**
 *  \file IMP/hdf5/restraint_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_RESTRAINT_IO_H
#define IMPHDF5_RESTRAINT_IO_H

#include "hdf5_config.h"
#include "NodeHandle.h"
#include <IMP/Restraint.h>

IMPHDF5_BEGIN_NAMESPACE
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
IMPHDF5EXPORT void add_restraint(RootHandle parent, Restraint *r);
/** Add the current score of the restraint to the given frame.*/
IMPHDF5EXPORT void save_frame(RootHandle parent, int frame, Restraint *r);

/** Return the list of particles for a restraint node. This is not
    such a nice interface, but I can't think of a better one at the
    moment.*/
IMPHDF5EXPORT ParticlesTemp get_restraint_particles(NodeHandle f,
                                                    int frame);
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPHDF5EXPORT ParticlesTemp get_restraint_particles(NodeHandle f,
                                                    NodeIDKeys &fks,
                                                    int frame);
#endif

/** Return the restraint score, if available (or -inf if it is not).
    This is not
    such a nice interface, but I can't think of a better one at the
    moment.*/
IMPHDF5EXPORT double get_restraint_score(NodeHandle f,
                                         int frame);

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPHDF5EXPORT double get_restraint_score(NodeHandle f,
                                         FloatKey &fk,
                                         int frame);
#endif

/** @} */
IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_RESTRAINT_IO_H */

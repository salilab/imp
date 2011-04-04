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

/** Add a restraint to the file.*/
IMPHDF5EXPORT void write_restraint(Restraint *r,
                                 RootHandle parent);
/** Add the current score of the restraint to the given frame.*/
IMPHDF5EXPORT void save_restraint(Restraint *r,
                                  RootHandle parent, int frame);

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
IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_RESTRAINT_IO_H */

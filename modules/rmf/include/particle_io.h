/**
 *  \file IMP/rmf/particle_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_PARTICLE_IO_H
#define IMPRMF_PARTICLE_IO_H

#include "rmf_config.h"
#include <RMF/NodeHandle.h>
#include <IMP/atom/Hierarchy.h>

IMPRMF_BEGIN_NAMESPACE

/** \name Particle I/O
    Arbitrary (non IMP::atom::Hierarchy) particles can be written to
    the file and read back from them. All non-IMP::Object attributes are
    supported. Note that currently particles must be added so that
    any particles that are store in a given particles attributes are
    added to the file first.

    The data is stored in a category named IMP.
    @{
*/
/** Add the passed particles to the RMF file. You can use the
    save_frame() function to write these and other linked objects
    to particular frames of the file.
 */
IMPRMFEXPORT void add_particles(RMF::FileHandle fh, const ParticlesTemp &ps);

/** Read all particles (as opposed to molecular hierarchy nodes).
 */
IMPRMFEXPORT ParticlesTemp create_particles(RMF::FileConstHandle fh, Model *m);

/** Link a set of particles with the particles that had been written to
    the file.
*/
IMPRMFEXPORT void link_particles(RMF::FileConstHandle fh,
                                 const ParticlesTemp &ps);
/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_PARTICLE_IO_H */

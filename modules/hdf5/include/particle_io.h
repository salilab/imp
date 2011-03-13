/**
 *  \file IMP/hdf5/particle_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_PARTICLE_IO_H
#define IMPHDF5_PARTICLE_IO_H

#include "hdf5_config.h"
#include "NodeHandle.h"
#include <IMP/atom/Hierarchy.h>

IMPHDF5_BEGIN_NAMESPACE

/**
*/
IMPHDF5EXPORT extern const KeyCategory IMP;

/** Write the attributes of an arbitrary particle to the file.
 */
IMPHDF5EXPORT void write_particle(Particle* ps, RootHandle fh);

/** Read all particles (as opposed to molecular hierarchy nodes)
 */
IMPHDF5EXPORT ParticlesTemp read_all_particles(RootHandle fh, Model *m);

IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_PARTICLE_IO_H */

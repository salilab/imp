/**
 *  \file IMP/kernel/io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_IO_H
#define IMPKERNEL_IO_H

#include <IMP/kernel/kernel_config.h>
#include "file.h"
#include "OptimizerState.h"
#include "internal/utility.h"
#include <boost/format.hpp>

IMPKERNEL_BEGIN_NAMESPACE

/** \name Buffer I/O
    Write/read the state of the particles to/from a buffer in memory.
    \note Not all particles need to have all the attributes,
    missing attributes will be skipped. However, the set of attributes
    must match on the write and read particles.

    \note There is no handling of architectural issues. That is, this
    is only guaranteed to work if it is read and written on the same
    operating system and system bit length. We could probably fix this.

    \note both these methods should be considered unstable.
    @{
*/
//! return a binary buffer with the data
IMPKERNELEXPORT base::Vector<char>
write_particles_to_buffer(const ParticlesTemp &particles,
                          const FloatKeys &keys);
//! load found attributes into the particles
IMPKERNELEXPORT void
read_particles_from_buffer( const base::Vector<char> &buffer,
                            const ParticlesTemp &particles,
                            const FloatKeys &keys);

/** @} */

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_IO_H */

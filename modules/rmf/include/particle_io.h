/**
 *  \file IMP/rmf/particle_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_PARTICLE_IO_H
#define IMPRMF_PARTICLE_IO_H

#include <IMP/rmf/rmf_config.h>
#include "link_macros.h"
#include <IMP/atom/Hierarchy.h>

IMPRMF_BEGIN_NAMESPACE

/** \name Particle I/O
    Arbitrary (non IMP::atom::Hierarchy) particles can be written to
    the file and read back from them. All non IMP::base::Object attributes are
    supported. Note that currently particles must be added so that
    any particles that are stored in a given particles attributes are
    added to the file first.

    The data is stored in a category named IMP.
    @{
*/
IMP_DECLARE_LINKERS(Particle, particle, particles, Particle *, ParticlesTemp,
                    Particle *, ParticlesTemp,
                    (RMF::FileConstHandle fh, Model *m), );

/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_PARTICLE_IO_H */

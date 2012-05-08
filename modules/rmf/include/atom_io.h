/**
 *  \file IMP/rmf/atom_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_IO_H
#define IMPRMF_ATOM_IO_H

#include "rmf_config.h"
#include "link_macros.h"
#include <IMP/atom/Hierarchy.h>

IMPRMF_BEGIN_NAMESPACE

/** \name Hierarchy I/O
    Hierarchy I/O writes IMP::atom::Hierarchy information along with
    the information contained in the following decorators
    - IMP::atom::Atom
    - IMP::atom::Residue
    - IMP::core::XYZR
    - IMP::atom::Mass
    - IMP::atom::Diffusion
    - IMP::core::Typed
    - IMP::display::Colored
    - IMP::atom::Domain
    - IMP::atom::Molecule
    - IMP::atom::Copy
    - IMP::core::RigidBody

    \note The handling of rigid bodies is not entirely straight forward and
    might have some side effects. When loading them, the orientation of the
    rigid body is read from the file as well as the coordinates of the members.
    The internal coordinats of the members are then updated that so their
    coordinates, the pose of the rigid body and the internal coordinates are
    all in sync. This is to avoid issues with rigid bodies not always being
    created consistently by \imp (since the principal axes of the body may
    be numerically unstable).
    @{
 */

IMP_DECLARE_LINKERS(Hierarchy, hierarchy, hierarchies,
                    atom::Hierarchy,atom::Hierarchies,
                    atom::Hierarchy,atom::Hierarchies,
                    (RMF::FileHandle fh),
                    (RMF::FileConstHandle fh, Model *m));
/** @} */



IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_IO_H */

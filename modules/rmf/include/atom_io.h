/**
 *  \file IMP/rmf/atom_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_IO_H
#define IMPRMF_ATOM_IO_H

#include <IMP/rmf/rmf_config.h>
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
    - IMP::core::RigidMember (rigid body information will be written and rigid
    bodies created even when the IMP::core::RigidBody is not itself written
    to the file)
    - IMP::core::RigidBody

    @note IMP::core::XYZ and IMP::core::RigidBody data are stored at each frame.
    Data for all the other decorators are assumed to be static attributes, and
    are stored once for the entire RMF file. There is currently no way to
    customize this. Complain to the IMP team if this is problematic.

    \anchor topology_matching
    \note To avoid unexpected behavior, be aware that when linking
    atom::Hierarchies against an RMF file, the linked hierarchies are
    assumed to be identical in all of their static attributes to the
    hierarchies in the RMF file (within
    RMF::REPRESENTATION nodes). In particular, these hierachies must
    have the same tree topology (number of children for each node).

    @{
 */

IMP_DECLARE_LINKERS(Hierarchy, hierarchy, hierarchies,
                    atom::Hierarchy,atom::Hierarchies,
                    atom::Hierarchy,atom::Hierarchies,
                    (RMF::FileConstHandle fh, Model *m),
                    See \ref topology_matching "Linking hierarchies" for more
                    information and intricacies);

/** Set whether the forces (and torques) are saved to the file.*/
IMPRMFEXPORT void set_save_forces(RMF::FileHandle fh, bool tf);

/** @} */



IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_IO_H */

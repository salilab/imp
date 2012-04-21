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

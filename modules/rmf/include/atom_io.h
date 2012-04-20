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
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/OptimizerState.h>

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
IMPRMFEXPORT void add_hierarchies(RMF::FileHandle fh,
                                  const atom::Hierarchies& hs);
#ifndef IMP_DOXYGEN
IMPRMFEXPORT void add_hierarchy(RMF::FileHandle fh, atom::Hierarchy hs);
#endif

/** Create a hierarchy from part of the file.
 */
IMPRMFEXPORT atom::Hierarchies create_hierarchies(RMF::FileConstHandle fh,
                                                  Model *m);


/** Associate the passed hierarchies with all the hierarchies in the file.
    They must match exactly.

    If overwrite is false, then it is an error if the file already is
    associated with something.
*/
IMPRMFEXPORT void link_hierarchies(RMF::FileConstHandle rh,
                                  atom::Hierarchies hs,
                                  bool overwrite=false);
/** @} */



IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_IO_H */

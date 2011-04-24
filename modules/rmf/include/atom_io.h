/**
 *  \file IMP/rmf/atom_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_IO_H
#define IMPRMF_ATOM_IO_H

#include "rmf_config.h"
#include "NodeHandle.h"
#include "RootHandle.h"
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

/** Write the hierarchies as a file with data store in the frame. The
    hierarchies must have been previously read from the file if you are
    adding a frame.*/
IMPRMFEXPORT void save_frame(RootHandle fh,
                             unsigned int frame, atom::Hierarchy hs);

IMPRMFEXPORT void add_hierarchy(RootHandle fh, atom::Hierarchy hs);


/** Create a hierarchy from part of the file.
 */
IMPRMFEXPORT atom::Hierarchies create_hierarchies(RootHandle fh, Model *m);


/** Associate the passed hierarchies with all the hierarchies in the file.
    They must match exactly.
*/
IMPRMFEXPORT void set_hierarchies(RootHandle rh,
                                   atom::Hierarchies hs);

/** Read the ith frame from a file as a new hierarchy.
 */
IMPRMFEXPORT void load_frame(RootHandle fh,
                              unsigned int frame,
                              atom::Hierarchy hs);
/** @} */


/** Periodically call the save_frame() method to save the hierarchy
    to the given file.
*/
class IMPRMFEXPORT SaveHierarchyConfigurationOptimizerState:
  public OptimizerState {
  atom::Hierarchies hs_;
  RootHandle fh_;
 public:
  SaveHierarchyConfigurationOptimizerState(atom::Hierarchies hs,
                                           RootHandle fh);
  IMP_PERIODIC_OPTIMIZER_STATE(SaveHierarchyConfigurationOptimizerState);
};


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_IO_H */

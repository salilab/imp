/**
 *  \file IMP/hdf5/atom_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_ATOM_IO_H
#define IMPHDF5_ATOM_IO_H

#include "hdf5_config.h"
#include "NodeHandle.h"
#include "RootHandle.h"
#include <IMP/atom/Hierarchy.h>
#include <IMP/OptimizerState.h>

IMPHDF5_BEGIN_NAMESPACE

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
IMPHDF5EXPORT void save_configuration(atom::Hierarchy hs, RootHandle fh,
                                     unsigned int frame);

IMPHDF5EXPORT void write_hierarchy(atom::Hierarchy hs, RootHandle fh);


/** Create a hierarchy from part of the file.
 */
IMPHDF5EXPORT atom::Hierarchies read_all_hierarchies(RootHandle fh, Model *m);

/** Read the ith frame from a file as a new hierarchy.
 */
IMPHDF5EXPORT void load_configuration(RootHandle fh,
                                      atom::Hierarchy hs,
                                      unsigned int frame);
/** @} */

class IMPHDF5EXPORT SaveHierarchyConfigurationOptimizerState:
  public OptimizerState {
  atom::Hierarchies hs_;
  RootHandle fh_;
 public:
  SaveHierarchyConfigurationOptimizerState(atom::Hierarchies hs,
                                           RootHandle fh);
  IMP_PERIODIC_OPTIMIZER_STATE(SaveHierarchyConfigurationOptimizerState);
};


IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_ATOM_IO_H */

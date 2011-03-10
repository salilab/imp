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
#include <IMP/atom/Hierarchy.h>

IMPHDF5_BEGIN_NAMESPACE

/** Write the hierarchies as a file with data store in the frame. The
    hierarchies must have been previously read from the file if you are
    adding a frame.*/
IMPHDF5EXPORT void save_conformation(atom::Hierarchy hs, RootHandle fh,
                                     unsigned int frame);

IMPHDF5EXPORT void write_hierarchy(atom::Hierarchy hs, RootHandle fh);


/** Create a hierarchy from part of the file.
 */
IMPHDF5EXPORT atom::Hierarchies read_all_hierarchies(RootHandle fh, Model *m);

/** Read the ith frame from a file as a new hierarchy.
 */
IMPHDF5EXPORT void load_conformation(RootHandle fh,
                                               atom::Hierarchy hs,
                                               unsigned int frame);

IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_ATOM_IO_H */

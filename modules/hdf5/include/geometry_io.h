/**
 *  \file IMP/hdf5/geometry_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_GEOMETRY_IO_H
#define IMPHDF5_GEOMETRY_IO_H

#include "hdf5_config.h"
#include "NodeHandle.h"
#include <IMP/display/Writer.h>
#include <IMP/display/display_macros.h>

IMPHDF5_BEGIN_NAMESPACE

/** Add the geometry to the file */
IMPHDF5EXPORT void write_geometry(display::Geometry *r,
                                RootHandle parent);
/** Save the geometry for the specified frame. */
IMPHDF5EXPORT void save_conformation(display::Geometry *r,
                                     RootHandle parent, int frame);


IMPHDF5EXPORT display::Geometries read_all_geometries(RootHandle parent,
                                                      int frame);
IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_GEOMETRY_IO_H */

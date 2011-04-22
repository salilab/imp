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

/** \name Geometry I/O

    The geometry I/O support currently handles geometry composed of
    - IMP::display::SegmentGeometry
    - IMP::display::CylinderGeometry
    - IMP::display::SphereGeometry
    - IMP::display::SurfaceMeshGeometry

    Other types can be supported when requested. Be aware, many
    more complex geometry types are automatically decomposed into
    the above types and so, more or less, supported.
    @{
 */
/** Add the geometry to the file */
IMPHDF5EXPORT void add_geometry(RootHandle parent, display::Geometry *r);
/** Save the geometry for the specified frame. */
IMPHDF5EXPORT void save_frame(RootHandle parent, int frame,
                              display::Geometry *r);


IMPHDF5EXPORT display::Geometries create_geometries(RootHandle parent,
                                                        int frame);
/** @} */
IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_GEOMETRY_IO_H */

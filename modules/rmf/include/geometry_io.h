/**
 *  \file IMP/rmf/geometry_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_GEOMETRY_IO_H
#define IMPRMF_GEOMETRY_IO_H

#include "rmf_config.h"
#include <IMP/display/declare_Geometry.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>

IMPRMF_BEGIN_NAMESPACE

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
IMPRMFEXPORT void add_geometries(RMF::FileHandle parent,
                                 const display::GeometriesTemp &r);
#ifndef IMP_DOXYGEN
IMPRMFEXPORT void add_geometry(RMF::FileHandle parent, display::Geometry *r);
#endif

IMPRMFEXPORT display::Geometries create_geometries(RMF::FileConstHandle parent);

IMPRMFEXPORT void link_geometries(RMF::FileConstHandle parent,
                                  const display::GeometriesTemp &r);
/** @} */
IMPRMF_END_NAMESPACE

#endif /* IMPRMF_GEOMETRY_IO_H */

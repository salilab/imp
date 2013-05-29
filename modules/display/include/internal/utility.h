/**
 *  \file display/internal/utility.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_INTERNAL_UTILITY_H
#define IMPDISPLAY_INTERNAL_UTILITY_H

#include <IMP/display/display_config.h>
#include <IMP/algebra/Vector3D.h>

IMPDISPLAY_BEGIN_NAMESPACE
class SurfaceMeshGeometry;
class PolygonGeometry;
IMPDISPLAY_END_NAMESPACE

IMPDISPLAY_BEGIN_INTERNAL_NAMESPACE
IMPDISPLAYEXPORT Ints get_triangles(PolygonGeometry *sg);

IMPDISPLAYEXPORT Ints get_triangles(SurfaceMeshGeometry *sg);

IMPDISPLAYEXPORT algebra::Vector3Ds get_normals(
    const Ints &faces, const algebra::Vector3Ds &vertices);

IMPDISPLAY_END_INTERNAL_NAMESPACE

#endif /* IMPDISPLAY_INTERNAL_UTILITY_H */

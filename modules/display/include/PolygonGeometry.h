/**
 *  \file PolygonGeometry.h
 *  \brief Implement PolygonGeometry
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_POLYGON_GEOMETRY_H
#define IMPDISPLAY_POLYGON_GEOMETRY_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include "geometry.h"

IMPDISPLAY_BEGIN_NAMESPACE

//! Render a polygon.
/** The polygon should be simple, and, to be completely safe, convex.
  This latter can be fixed if needed.
 */
class IMPDISPLAYEXPORT PolygonGeometry: public Geometry
{
  algebra::Vector3Ds pts_;
public:
  //! The vertices of the polygon in counter clockwise order
  PolygonGeometry(const algebra::Vector3Ds &pts);

  IMP_GEOMETRY(PolygonGeometry, internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_POLYGON_GEOMETRY_H */

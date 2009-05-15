/**
 *  \file CylinderGeometry.h
 *  \brief Represent an Cylinder particle with a sphere
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CYLINDER_GEOMETRY_H
#define IMPDISPLAY_CYLINDER_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include "geometry.h"
#include <IMP/algebra/Cylinder3D.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a static cylinder (or segment)
/**
 */
class IMPDISPLAYEXPORT CylinderGeometry: public Geometry
{
  algebra::Cylinder3D c_;
public:
  //! Create a static cylinder or segment
  /** If the radius is 0, it is a segment. If the color is skipped,
      the default color (gray) is used.
  */
  CylinderGeometry(const algebra::Cylinder3D &c);

  IMP_GEOMETRY(CylinderGeometry, internal::version_info)
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CYLINDER_GEOMETRY_H */

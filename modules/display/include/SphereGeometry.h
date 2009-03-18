/**
 *  \file SphereGeometry.h
 *  \brief Represent an SphereDecorator particle with a sphere
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_SPHERE_GEOMETRY_H
#define IMPDISPLAY_SPHERE_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include "geometry.h"
#include <IMP/algebra/Sphere3D.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a static sphere (or point)
/**
 */
class IMPDISPLAYEXPORT SphereGeometry: public Geometry
{
  algebra::Sphere3D s_;
public:
  //! Create a static sphere or point
  /** If the radius is skipped, it is a point. If the color is skipped,
      the default color (gray) is used.
  */
  SphereGeometry(const algebra::Sphere3D &s);

  virtual ~SphereGeometry();

  virtual Float get_size() const {
    return s_.get_radius();
  }


  IMP_GEOMETRY(internal::version_info)
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_SPHERE_GEOMETRY_H */

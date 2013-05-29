/**
 *  \file IMP/display/GeometrySet.h
 *  \brief Implement geometry for the basic shapes from IMP.algebra.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_SET_H
#define IMPDISPLAY_GEOMETRY_SET_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "geometry_macros.h"

IMPDISPLAY_BEGIN_NAMESPACE

/** \brief Group of set of geometric elements
 */
class IMPDISPLAYEXPORT GeometrySet : public Geometry {
  Geometries v_;

 public:
  GeometrySet(const Geometries &v);
  GeometrySet(const Geometries &v, const Color &c);
  GeometrySet(const Geometries &v, const std::string n);
  GeometrySet(const Geometries &v, const Color &c, std::string n);
  Geometries get_geometry() const { return v_; }
  IMP_GEOMETRY(GeometrySet);
};

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_GEOMETRY_SET_H */

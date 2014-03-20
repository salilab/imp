/**
 *  \file GeometryProcessor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/GeometryProcessor.h"
#include <boost/algorithm/string/predicate.hpp>
#include <IMP/base/Pointer.h>

IMPDISPLAY_BEGIN_NAMESPACE

#define GP_HANDLE(Name, nm)                                 \
  {                                                         \
    Name##Geometry *sg = dynamic_cast<Name##Geometry *>(g); \
    if (sg) {                                               \
      if (handle_##nm(sg, c, name)) {                       \
        return;                                             \
      }                                                     \
    }                                                       \
  }

void GeometryProcessor::handle_geometry_internal(Geometry *g, bool has_color,
                                                 Color c, std::string name) {
  IMP::base::PointerMember<Geometry> pg(g);
  // use own color instead of default whenever it is specified
  if (g->get_has_color()) {
    c = g->get_color();
    has_color = true;
  }
  if (name.empty()) {
    name = g->get_name();
  }
  // try primitive geometries first
  GP_HANDLE(Sphere, sphere);
  GP_HANDLE(Cylinder, cylinder);
  GP_HANDLE(Ellipsoid, ellipsoid);
  GP_HANDLE(BoundingBox, bounding_box);
  GP_HANDLE(Point, point);
  GP_HANDLE(Segment, segment);
  GP_HANDLE(Polygon, polygon);
  GP_HANDLE(Triangle, triangle);
  GP_HANDLE(Label, label);
  GP_HANDLE(SurfaceMesh, surface_mesh);
  GP_HANDLE(, anything);
  // if primitives didn't work, try decompose to components
  Geometries comp = g->get_components();
  if (comp.size() == 1 && comp[0] == g) {
    IMP_THROW("Couldn't handle " << Showable(g), base::ValueException);
  } else {
    for (unsigned int i = 0; i < comp.size(); ++i) {
      handle_geometry_internal(comp[i], has_color, c, name);
    }
  }
}

void GeometryProcessor::handle_geometry(Geometry *g) {
  handle_geometry_internal(g, false, Color(.7, .7, .7), std::string());
}

IMPDISPLAY_END_NAMESPACE

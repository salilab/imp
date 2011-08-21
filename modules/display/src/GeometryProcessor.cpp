/**
 *  \file GeometryProcessor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/GeometryProcessor.h"
#include <boost/algorithm/string/predicate.hpp>


IMPDISPLAY_BEGIN_NAMESPACE

#define GP_HANDLE(Name) {                               \
    Name *sg= dynamic_cast<Name*>(g);                   \
    if (sg) {                                           \
      if (handle(sg, c, name)) {                        \
        return;                                         \
      }                                                 \
    }                                                   \
  }                                                     \

void GeometryProcessor::handle_geometry_internal(Geometry* g,
                                                  bool has_color,
                                                  Color c,
                                                  std::string name) {
  IMP::OwnerPointer<Geometry> pg(g);
  if (!has_color && g->get_has_color()) {
    c= g->get_color();
    has_color=true;
  }
  if (name.empty()) {
    name= g->get_name();
  }
  GP_HANDLE(SphereGeometry);
  GP_HANDLE(CylinderGeometry);
  GP_HANDLE(EllipsoidGeometry);
  GP_HANDLE(BoundingBoxGeometry);
  GP_HANDLE(PointGeometry);
  GP_HANDLE(SegmentGeometry);
  GP_HANDLE(PolygonGeometry);
  GP_HANDLE(TriangleGeometry);
  GP_HANDLE(LabelGeometry);
  GP_HANDLE(SurfaceMeshGeometry);
  GP_HANDLE(Geometry);
  Geometries comp= g->get_components();
  if (comp.size()==1 && comp[0]== g) {
    IMP_THROW("Couldn't handle " << *g,
              ValueException);
  } else {
    for (unsigned int i=0; i< comp.size(); ++i) {
      handle_geometry_internal(comp[i], has_color, c, name);
    }
  }
}

void GeometryProcessor::handle_geometry(Geometry *g) {
  handle_geometry_internal(g, false, Color(.7, .7, .7), std::string());
}



IMPDISPLAY_END_NAMESPACE

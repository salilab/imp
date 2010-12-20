/**
 *  \file GeometryProcessor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/GeometryProcessor.h"
#include <boost/algorithm/string/predicate.hpp>


IMPDISPLAY_BEGIN_NAMESPACE

#define HANDLE(Name) {                                  \
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
  IMP::internal::OwnerPointer<Geometry> pg(g);
  if (!has_color && g->get_has_color()) {
    c= g->get_color();
    has_color=true;
  }
  if (name.empty()) {
    name= g->get_name();
  }
  HANDLE(SphereGeometry);
  HANDLE(CylinderGeometry);
  HANDLE(EllipsoidGeometry);
  HANDLE(BoundingBoxGeometry);
  HANDLE(PointGeometry);
  HANDLE(SegmentGeometry);
  HANDLE(PolygonGeometry);
  HANDLE(TriangleGeometry);
  HANDLE(LabelGeometry);
  HANDLE(Geometry);
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

/**
 *  \file GeometryProcessor.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/GeometryProcessor.h"
#include <boost/algorithm/string/predicate.hpp>


IMPDISPLAY_BEGIN_NAMESPACE

#define HANDLE(Name) {                                  \
    Name *sg= dynamic_cast<Name*>(g);                   \
    if (sg) {                                           \
      if (process(sg, c, name)) {                       \
        IMP_LOG(VERBOSE, "Geometry \"" << g->get_name() \
                << "\" of type " << #Name               \
                << " handed." << std::endl);            \
        return;                                         \
      } else {                                          \
        IMP_LOG(VERBOSE, "Geometry \"" << g->get_name() \
                << "\" of type " << #Name               \
                << " not handled." << std::endl);       \
      }                                                 \
    }                                                   \
  }                                                     \

void GeometryProcessor::process_geometry_internal(Geometry* g,
                                                  bool has_color,
                                                  Color c,
                                                  std::string name) {
  IMP::internal::OwnerPointer<Geometry> pg(g);
  if (!has_color && g->get_has_color()) {
    c= g->get_color();
    has_color=true;
  }
  if (name != std::string()) {
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
  HANDLE(Geometry);
  Geometries comp= g->get_components();
  if (comp.size()==1 && comp[0]== g) {
    IMP_THROW("Couldn't handle " << *g,
              UsageException);
  } else {
    for (unsigned int i=0; i< comp.size(); ++i) {
      process_geometry_internal(comp[i], has_color, c, name);
    }
  }
}

void GeometryProcessor::process_geometry(Geometry *g) {
  process_geometry_internal(g, false, Color(.7, .7, .7), std::string());
}



IMPDISPLAY_END_NAMESPACE

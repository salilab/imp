/**
 *  \file IMP/display/GeometryProcessor.h
 *  \brief Implement GeometryProcessor
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_PROCESSOR_H
#define IMPDISPLAY_GEOMETRY_PROCESSOR_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "primitive_geometries.h"

IMPDISPLAY_BEGIN_NAMESPACE

//! Provide a standard geometry processing framework.
/** This class is designed to be used as a mixin, so
    inherit from this and overload various process methods
    as needed.
 */
class IMPDISPLAYEXPORT GeometryProcessor
{
protected:
  ~GeometryProcessor(){}

  virtual bool handle_sphere(SphereGeometry *,
                        Color, std::string) {
                         return false;
                       }
  virtual bool handle_cylinder(CylinderGeometry *,
                               Color, std::string) {
    return false;
  }
  virtual bool handle_point(PointGeometry *,
                            Color, std::string) {
    return false;
  }
  virtual bool handle_segment(SegmentGeometry *,
                              Color, std::string) {
    return false;
  }
  virtual bool handle_polygon(PolygonGeometry *,
                              Color, std::string) {
    return false;
  }
  virtual bool handle_triangle(TriangleGeometry *,
                               Color, std::string) {
    return false;
  }
  virtual bool handle_surface_mesh(SurfaceMeshGeometry *,
                                   Color, std::string) {
    return false;
  }
  virtual bool handle_label(LabelGeometry *,
                            Color, std::string) {
    return false;
  }
  virtual bool handle_ellipsoid(EllipsoidGeometry *,
                                Color, std::string) {
    return false;
  }
  virtual bool handle_bounding_box(BoundingBoxGeometry *, Color,
                                   std::string) {
    return false;
  }
  virtual bool handle_anything(Geometry *, Color, std::string) {
    return false;
  }
  void handle_geometry (Geometry* g);
 private:
  void handle_geometry_internal(Geometry* g, bool has_color,
                                 Color c,
                                 std::string);
};
#ifndef IMP_DOXYGEN
typedef base::Vector<GeometryProcessor> GeometryProcessors;
#endif
IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_PROCESSOR_H */

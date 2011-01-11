/**
 *  \file GeometryProcessor.h
 *  \brief Implement GeometryProcessor
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_PROCESSOR_H
#define IMPDISPLAY_GEOMETRY_PROCESSOR_H

#include "display_config.h"
#include "geometry.h"

IMPDISPLAY_BEGIN_NAMESPACE

//! Provide a standard geometry processing framework.
/** To use, inherit from this and overload various process methods
    as needed.
 */
class IMPDISPLAYEXPORT GeometryProcessor
{
protected:
  virtual ~GeometryProcessor() {}

  virtual bool handle(SphereGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(CylinderGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(PointGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(SegmentGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(PolygonGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(TriangleGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(LabelGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(EllipsoidGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool handle(BoundingBoxGeometry *, Color,
                       std::string) {
    return false;
  }
  virtual bool handle(Geometry *, Color, std::string) {
    return false;
  }
  void handle_geometry(Geometry* g);
private:
  void handle_geometry_internal(Geometry* g, bool has_color,
                                 Color c,
                                 std::string);
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_PROCESSOR_H */

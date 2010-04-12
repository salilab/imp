/**
 *  \file GeometryProcessor.h
 *  \brief Implement GeometryProcessor
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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

  virtual bool process(SphereGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(CylinderGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(PointGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(SegmentGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(PolygonGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(TriangleGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(EllipsoidGeometry *,
                       Color, std::string) {
    return false;
  }
  virtual bool process(BoundingBoxGeometry *, Color,
                       std::string) {
    return false;
  }
  virtual bool process(Geometry *, Color, std::string) {
    return false;
  }
  void process_geometry(Geometry* g);
private:
  void process_geometry_internal(Geometry* g, bool has_color,
                                 Color c,
                                 std::string);
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_PROCESSOR_H */

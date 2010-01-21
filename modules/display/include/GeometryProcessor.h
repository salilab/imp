/**
 *  \file GeometryProcessor.h
 *  \brief Implement GeometryProcessor
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_PROCESSOR_H
#define IMPDISPLAY_GEOMETRY_PROCESSOR_H

#include "config.h"
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

  virtual bool process(SphereGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(CylinderGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(PointGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(SegmentGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(PolygonGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(TriangleGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(EllipsoidGeometry *g,
                       Color color, std::string name) {
    return false;
  }
  virtual bool process(BoundingBoxGeometry *g, Color color,
                       std::string name) {
    return false;
  }
  virtual bool process(Geometry *g, Color color, std::string name) {
    return false;
  }
  void process_geometry(Geometry* g);
private:
  void process_geometry_internal(Geometry* g, bool has_color,
                                 Color c,
                                 std::string name);
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_PROCESSOR_H */

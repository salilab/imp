/**
 *  \file FilterGeometry.h
 *  \brief Implement FilterGeometry
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_FILTER_GEOMETRY_H
#define IMPDISPLAY_FILTER_GEOMETRY_H

#include "config.h"
#include "geometry.h"
#include "GeometryProcessor.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Plane3D.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Remove geometry which is not above a plane.
/** Any geometry which is not above the plane is not passed on to the writer
    or log or whatever this CompoundGoemetry is passed to.
 */
class IMPDISPLAYEXPORT FilterGeometry: public GeometryProcessor,
                                       public Geometry
{
  const algebra::Plane3D &p_;
  Geometries gdata_;
  mutable Geometries filtered_;
 protected:
  using GeometryProcessor::process;
  bool process(SphereGeometry *g,
                         Color color, std::string name);
  bool process(CylinderGeometry *g,
                       Color color, std::string name);
  bool process(PointGeometry *g,
                       Color color, std::string name);
  bool process(SegmentGeometry *g,
                       Color color, std::string name);
public:
  //! Pay attention to the orientation of the plane.
  FilterGeometry(const algebra::Plane3D &p);

  void add_geometry(Geometry* g);

  void add_geometry(const Geometries& g);

  IMP_GEOMETRY(FilterGeometry)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_FILTER_GEOMETRY_H */

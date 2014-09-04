/**
 *  \file IMP/display/FilterGeometry.h
 *  \brief Remove geometry which is not above a plane.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_FILTER_GEOMETRY_H
#define IMPDISPLAY_FILTER_GEOMETRY_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "GeometryProcessor.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Plane3D.h>
#include <IMP/base/utility_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Remove geometry which is not above a plane.
/** Any geometry which is not above the plane is not passed on to the writer
    or log or whatever this CompoundGeometry is passed to.
 */
class IMPDISPLAYEXPORT FilterGeometry : public GeometryProcessor,
                                        public Geometry {
  const algebra::Plane3D &p_;
  Geometries gdata_;
  mutable Geometries filtered_;
  bool handle_sphere(SphereGeometry *g, Color color, std::string name);
  bool handle_cylinder(CylinderGeometry *g, Color color, std::string name);
  bool handle_point(PointGeometry *g, Color color, std::string name);
  bool handle_segment(SegmentGeometry *g, Color color, std::string name);

 public:
  //! Pay attention to the orientation of the plane.
  FilterGeometry(const algebra::Plane3D &p);

  void add_geometry(Geometry *g);

  void add_geometry(const Geometries &g);

  virtual IMP::display::Geometries get_components() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(FilterGeometry);
};

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_FILTER_GEOMETRY_H */

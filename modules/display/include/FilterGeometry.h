/**
 *  \file FilterGeometry.h
 *  \brief Implement FilterGeometry
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_FILTER_GEOMETRY_H
#define IMPDISPLAY_FILTER_GEOMETRY_H

#include "config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Plane3D.h>
#include "geometry.h"

IMPDISPLAY_BEGIN_NAMESPACE

//! Remove geometry which is not above a plane.
/** Any geometry which is not above the plane is not passed on to the writer
    or log or whatever this CompoundGoemetry is passed to.
 */
class IMPDISPLAYEXPORT FilterGeometry: public CompoundGeometry
{
  const algebra::Plane3D &p_;
  CompoundGeometries edata_;
  Geometries gdata_;

public:
  //! Pay attention to the orientation of the plane.
  FilterGeometry(const algebra::Plane3D &p);

  void add_geometry(CompoundGeometry* g);

  void add_geometry(Geometry* g);

  void add_geometry(const CompoundGeometries& g);

  void add_geometry(const Geometries& g);

  IMP_COMPOUND_GEOMETRY(FilterGeometry, get_module_version_info())
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_FILTER_GEOMETRY_H */

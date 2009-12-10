/**
 *  \file geometry.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_H
#define IMPDISPLAY_GEOMETRY_H

#include "config.h"
#include "Color.h"
#include "macros.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RefCounted.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Cylinder3D.h>
#include <IMP/algebra/Ellipsoid3D.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPDISPLAY_BEGIN_NAMESPACE
class Geometry;
typedef VectorOfRefCounted<Geometry*> Geometries;
typedef std::vector<Geometry*> GeometriesTemp;


//! The base class for geometry.
/** This class doesn't have much to say other than the color.
 */
class IMPDISPLAYEXPORT Geometry: public Object
{
  bool has_color_;
  Color color_;
public:
  //Geometry();
  Geometry(std::string name);
  Geometry(Color c, std::string name);
  //Geometry(Color c);
  virtual Color get_color() const {
    IMP_USAGE_CHECK(has_color_, "Color not set", UsageException);
    return color_;
  }

  virtual bool get_has_color() const {
    return has_color_;
  }

  void set_color(Color c) {
    has_color_=true;
      color_=c;
  }

  //! Return a set of geometry composing this one
  virtual Geometries get_components() const {return GeometriesTemp();}

  IMP_REF_COUNTED_DESTRUCTOR(Geometry);
};

IMP_DISPLAY_GEOMETRY_DECL(SphereGeometry, algebra::Sphere3D);
IMP_DISPLAY_GEOMETRY_DECL(CylinderGeometry, algebra::Cylinder3D);
IMP_DISPLAY_GEOMETRY_DECL(EllipsoidGeometry, algebra::Ellipsoid3D);

IMP_DISPLAY_GEOMETRY_DECL(PointGeometry, algebra::Vector3D);
IMP_DISPLAY_GEOMETRY_DECL(SegmentGeometry, algebra::Segment3D);
IMP_DISPLAY_GEOMETRY_DECL(PolygonGeometry,
                          std::vector<algebra::Vector3D>);
IMP_DISPLAY_GEOMETRY_DECL(TriangleGeometry,
                          std::vector<algebra::Vector3D>);

IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(BoundingBoxGeometry,
                                       algebra::BoundingBoxD<3>);


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_H */

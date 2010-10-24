/**
 *  \file geometry.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_H
#define IMPDISPLAY_GEOMETRY_H

#include "display_config.h"
#include "Color.h"
#include "display_macros.h"
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
    IMP_USAGE_CHECK(has_color_, "Color not set");
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

IMP_DISPLAY_GEOMETRY_DECL(SphereGeometry, algebra::SphereD<3>);
IMP_DISPLAY_GEOMETRY_DECL(CylinderGeometry, algebra::Cylinder3D);
IMP_DISPLAY_GEOMETRY_DECL(EllipsoidGeometry, algebra::Ellipsoid3D);

IMP_DISPLAY_GEOMETRY_DECL(PointGeometry, algebra::VectorD<3>);
IMP_DISPLAY_GEOMETRY_DECL(SegmentGeometry, algebra::Segment3D);
/** If CGAL is available, then any simple, planar polygon can be
    used. Otherwise, the polygons should be simple, planar and convex.
*/
IMP_DISPLAY_GEOMETRY_DECL(PolygonGeometry,
                          std::vector<algebra::VectorD<3> >);
IMP_DISPLAY_GEOMETRY_DECL(TriangleGeometry,
                          std::vector<algebra::VectorD<3> >);

IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(BoundingBoxGeometry,
                                       algebra::BoundingBoxD<3>);

//! A text label for a ball in space
/* You can use the offset if the thing being labeled has a radius.
 */
class IMPDISPLAYEXPORT LabelGeometry: public Geometry {
  algebra::Sphere3D loc_;
  std::string text_;
 public:
  LabelGeometry(const algebra::Sphere3D &loc,
                std::string text);
  LabelGeometry(const algebra::Vector3D &loc,
                std::string text);
  std::string get_text() const {return text_;}
  const algebra::Sphere3D& get_location() const {return loc_;}
  IMP_GEOMETRY(LabelGeometry);
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_H */

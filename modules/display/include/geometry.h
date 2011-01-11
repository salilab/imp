/**
 *  \file geometry.h
 *  \brief Implement geometry for the basic shapes from IMP.algebra.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
#include <IMP/algebra/Plane3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>

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

/** \brief Display a sphere.
*/
IMP_DISPLAY_GEOMETRY_DECL(SphereGeometry, algebra::SphereD<3>);
/** \brief Display a cylinder.
*/
IMP_DISPLAY_GEOMETRY_DECL(CylinderGeometry, algebra::Cylinder3D);
/** \brief Display a ellipsoid.
*/
IMP_DISPLAY_GEOMETRY_DECL(EllipsoidGeometry, algebra::Ellipsoid3D);
/** \brief Display a point.
*/
IMP_DISPLAY_GEOMETRY_DECL(PointGeometry, algebra::VectorD<3>);
/** \brief Display a segment.
*/
IMP_DISPLAY_GEOMETRY_DECL(SegmentGeometry, algebra::Segment3D);
/** If CGAL is available, then any simple, planar polygon can be
    used. Otherwise, the polygons should be simple, planar and convex.
*/
IMP_DISPLAY_GEOMETRY_DECL(PolygonGeometry,
                          std::vector<algebra::VectorD<3> >);
/** \brief Display a triangule.
*/
IMP_DISPLAY_GEOMETRY_DECL(TriangleGeometry,
                          std::vector<algebra::VectorD<3> >);
/** \brief Display a bounding box.
*/
IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(BoundingBoxGeometry,
                                       algebra::BoundingBoxD<3>);

/** \brief Display a reference frame.

    x-axis is red, y is green and z is blue.
*/
IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(ReferenceFrameGeometry,
                                       algebra::ReferenceFrame3D);


//! A text label for a ball in space
/** You can use the offset if the thing being labeled has a radius.
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

#ifdef IMP_DISPLAY_USE_IMP_CGAL
//! Display a plane as truncated to a bounding box
/** This requires CGAL.
 */
class IMPDISPLAYEXPORT PlaneGeometry: public Geometry {
  algebra::Plane3D plane_;
  algebra::BoundingBox3D bb_;
 public:
  PlaneGeometry(const algebra::Plane3D &loc,
                const algebra::BoundingBox3D& box);
  IMP_GEOMETRY(PlaneGeometry);
};
#endif


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_GEOMETRY_H */

/**
 *  \file Geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/geometry.h"
#ifdef IMP_USE_CGAL
#include "IMP/cgal/internal/polygons.h"
#endif

IMPDISPLAY_BEGIN_NAMESPACE

/*Geometry::Geometry( ): has_color_(false){
  }*/
Geometry::Geometry(std::string name): Object(name), has_color_(false){
}
Geometry::Geometry(Color c, std::string name): Object(name){
  set_color(c);
}
/*Geometry::Geometry(Color c){
  set_color(c);
  }*/


std::ostream &operator<<(std::ostream &out,
                         const std::vector<algebra::VectorD<3> > &pts) {
  for (unsigned int i=0; i< pts.size(); ++i) {
    out << pts[i] << ": ";
  }
  return out;
}


IMP_DISPLAY_GEOMETRY_DEF(SphereGeometry, algebra::SphereD<3>);
IMP_DISPLAY_GEOMETRY_DEF(CylinderGeometry, algebra::Cylinder3D);
IMP_DISPLAY_GEOMETRY_DEF(EllipsoidGeometry, algebra::Ellipsoid3D);

namespace {
  Geometries decompose_box(const algebra::BoundingBox3D &bb,
                           bool has_color,
                           Color color,
                           std::string name) {
    Geometries ret;
    algebra::Vector3Ds corners= algebra::get_vertices(bb);
    IntPairs edges= algebra::get_edges(bb);
    for (unsigned int i=0; i< 12; ++i) {
      SegmentGeometry *ncg=
        new SegmentGeometry(algebra::Segment3D(corners[edges[i].first],
                                               corners[edges[i].second]));
      ncg->set_name(name);
      if (has_color){
        ncg->set_color(color);
      }
      ret.push_back(ncg);
    }
    return ret;
  }
}


IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DEF(BoundingBoxGeometry,
                                      algebra::BoundingBox3D,
                                      {ret=decompose_box(*this,
                                                         get_has_color(),
                                                         get_has_color()?
                                                         get_color()
                                                         :Color(),
                                                         get_name());
                                     });

namespace {
  Geometries get_frame(const algebra::Transformation3D &tr) {
    algebra::Vector3D o=tr.get_transformed(algebra::Vector3D(0,0,0));
    algebra::Vector3D pts[]={tr.get_transformed(algebra::Vector3D(10,0,0)),
                             tr.get_transformed(algebra::Vector3D(0,10,0)),
                             tr.get_transformed(algebra::Vector3D(0,0,10))};
    Color colors[]= {Color(1,0,0), Color(0,1,0), Color(0,0,1)};
    Geometries ret;
    for (unsigned int i=0; i <3; ++i) {
      algebra::Segment3D s(o, pts[i]);
      ret.push_back(new CylinderGeometry(algebra::Cylinder3D(s, 1)));
      ret.back()->set_color(colors[i]);
    }
    return ret;
  }
}

IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DEF(ReferenceFrameGeometry,
                                      algebra::ReferenceFrame3D,
   { algebra::Transformation3D tr = this->get_transformation_to();
     return get_frame(tr);
   });


IMP_DISPLAY_GEOMETRY_DEF(PointGeometry, algebra::VectorD<3>);
IMP_DISPLAY_GEOMETRY_DEF(SegmentGeometry, algebra::Segment3D);
IMP_DISPLAY_GEOMETRY_DEF(PolygonGeometry, std::vector<algebra::VectorD<3> >);
IMP_DISPLAY_GEOMETRY_DEF(TriangleGeometry, std::vector<algebra::VectorD<3> >);


Geometries LabelGeometry::get_components() const {
  return Geometries(1, const_cast<LabelGeometry*>(this));
}

LabelGeometry::LabelGeometry(const algebra::Sphere3D &loc,
                             std::string text): Geometry(""),
                                                loc_(loc), text_(text){}
LabelGeometry::LabelGeometry(const algebra::Vector3D &loc,
                             std::string text): Geometry(""),
                                                loc_(loc, 0),
                                                text_(text){}

void LabelGeometry::do_show(std::ostream &out) const {
  out << "label: " << get_text() << std::endl;
}


#ifdef IMP_USE_CGAL

Geometries PlaneGeometry::get_components() const {
  std::vector<algebra::Vector3D> poly
    = cgal::internal::get_intersection(plane_.get_normal(),
                             plane_.get_distance_from_origin(),
                             bb_);
  return Geometries(new PolygonGeometry(poly));
}

PlaneGeometry::PlaneGeometry(const algebra::Plane3D &loc,
                             const algebra::BoundingBox3D &bb):
  Geometry(""),
  plane_(loc),
  bb_(bb){}

void PlaneGeometry::do_show(std::ostream &out) const {
  out << "plane: " << plane_ << std::endl;
}
#endif

IMPDISPLAY_END_NAMESPACE

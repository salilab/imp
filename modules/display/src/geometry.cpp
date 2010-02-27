/**
 *  \file Geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/geometry.h"


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

#define PICK(v, i) (bb.get_corner(vertices[v][i])[i])
namespace {
  Geometries decompose_box(const algebra::BoundingBox3D &bb,
                           bool has_color,
                           Color color,
                           std::string name) {
    Geometries ret;
    static const int vertices[8][3]={{0,0,0}, {0,0,1}, {0,1,0},
                                     {0,1,1}, {1,0,0}, {1,0,1},
                                     {1,1,0}, {1,1,1}};
    static const int edges[12][2]={{0,1}, {0,2}, {0,4},
                                 {1,3}, {1,5},
                                   {2,3}, {2,6},
                                   {3,7},
                                   {4,5}, {4,6},
                                   {5,7},
                                   {6,7}};
    for (unsigned int i=0; i< 12; ++i) {
      int v0=edges[i][0];
      int v1=edges[i][1];
      algebra::VectorD<3> omin(PICK(v0, 0),
                             PICK(v0, 1),
                             PICK(v0, 2));
      algebra::VectorD<3> omax(PICK(v1, 0),
                             PICK(v1, 1),
                             PICK(v1, 2));
      SegmentGeometry *ncg=
        new SegmentGeometry(algebra::Segment3D(omin,
                                               omax));
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


IMP_DISPLAY_GEOMETRY_DEF(PointGeometry, algebra::VectorD<3>);
IMP_DISPLAY_GEOMETRY_DEF(SegmentGeometry, algebra::Segment3D);
IMP_DISPLAY_GEOMETRY_DEF(PolygonGeometry, std::vector<algebra::VectorD<3> >);
IMP_DISPLAY_GEOMETRY_DEF(TriangleGeometry, std::vector<algebra::VectorD<3> >);


IMPDISPLAY_END_NAMESPACE

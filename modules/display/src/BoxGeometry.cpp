/**
 *  \file BoxGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/BoxGeometry.h"
#include "IMP/display/CylinderGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

BoxGeometry::BoxGeometry(const algebra::Vector3D &min,
                         const algebra::Vector3D &max,
                         const Color &color): bb_(min, max),
                                              color_(color){
  CompoundGeometry::set_name("Bounding Box");
}

BoxGeometry::BoxGeometry(const algebra::BoundingBox3D &bb,
                         const Color &color): bb_(bb),
                                              color_(color){
  CompoundGeometry::set_name("Bounding Box");
}

#define PICK(v, i) (bb_.get_corner(vertices[v][i])[i])

Geometries BoxGeometry::get_geometry() const {
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
  Geometries ret;
  double r=.01*(bb_.get_corner(1)[0] - bb_.get_corner(0)[0]);
  for (unsigned int i=0; i< 12; ++i) {
    int v0=edges[i][0];
    int v1=edges[i][1];
    /*std::cout << v0 << " " << v1 << std::endl;
    std::cout << vertices[v0][0] << " " << vertices[v0][1]
              << " " << vertices[v0][2] << std::endl;

    std::cout << vertices[v1][0] << " " << vertices[v1][1]
    << " " << vertices[v1][2] << std::endl;*/
    algebra::Vector3D omin(PICK(v0, 0),
                           PICK(v0, 1),
                           PICK(v0, 2));
    algebra::Vector3D omax(PICK(v1, 0),
                           PICK(v1, 1),
                           PICK(v1, 2));
    CylinderGeometry *ncg=
      new CylinderGeometry(algebra::Cylinder3D(algebra::Segment3D(omin,
                                                                  omax), r));
    ncg->set_name(get_name());
    if (color_ != Color()) {
      ncg->set_color(color_);
    }
    ret.push_back(ncg);
  }
  return ret;
}


void BoxGeometry::show(std::ostream &out) const {
  out << "BoxGeometry: " << bb_ << std::endl;
}

IMPDISPLAY_END_NAMESPACE

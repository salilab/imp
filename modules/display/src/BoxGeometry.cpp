/**
 *  \file BoxGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/BoxGeometry.h"
#include "IMP/display/CylinderGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

BoxGeometry::BoxGeometry(const algebra::Vector3D &min,
                         const algebra::Vector3D &max,
                         const Color &color): min_(min), max_(max),
                                              color_(color){
}

BoxGeometry::~BoxGeometry(){}


Geometries BoxGeometry::get_geometry() const {
  Geometries ret;
  for (unsigned int i=0; i< 3; ++i) {
    algebra::Vector3D omin=min_;
    algebra::Vector3D omax=max_;
    omin[i]= max_[i];
    omax[i]= min_[i];
    CylinderGeometry *ncg= new CylinderGeometry(min_, omin);
    CylinderGeometry *xcg= new CylinderGeometry(max_, omax);
    if (color_ != Color()) {
      ncg->set_color(color_);
      xcg->set_color(color_);
    }
    ret.push_back(ncg);
    ret.push_back(xcg);
  }
  return ret;
}


void BoxGeometry::show(std::ostream &out) const {
  out << "BoxGeometry: " << min_ << " " << max_ << std::endl;
}

IMPDISPLAY_END_NAMESPACE

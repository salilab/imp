/**
 *  \file core/utility.cpp
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "IMP/core/utility.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
IMPCORE_BEGIN_NAMESPACE

algebra::Vector3D get_centroid(const XYZs &ps) {
 algebra::Vector3D cen(0.0,0.0,0.0);
 for (XYZs::const_iterator it = ps.begin(); it != ps.end();it++) {
   cen = cen + it->get_coordinates();
 }
 return cen/ps.size();
}

algebra::BoundingBoxD<3> get_bounding_box(const XYZRs &ps) {
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< ps.size(); ++i) {
    bb+= algebra::get_bounding_box(ps[i].get_sphere());
  }
  IMP_LOG_VERBOSE( "Bounding box is " << bb << std::endl);
  return bb;
}


IMPCORE_END_NAMESPACE

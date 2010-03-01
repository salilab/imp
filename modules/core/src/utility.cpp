/**
 *  \file core/utility.cpp
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include "IMP/core/utility.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
IMPCORE_BEGIN_NAMESPACE

algebra::VectorD<3> get_centroid(const XYZsTemp &ps) {
 algebra::VectorD<3> cen(0.0,0.0,0.0);
 for (XYZsTemp::const_iterator it = ps.begin(); it != ps.end();it++) {
   cen = cen + it->get_coordinates();
 }
 return cen/ps.size();
}

algebra::Segment3D get_diameter(const XYZsTemp &ps) {
  IMP_USAGE_CHECK(!ps.empty(), "Can't compute the diameter if you don't give "
            "me any particles.");
  XYZ p1,p2;
  double max_squared_dist=0.0;

  for (XYZsTemp::const_iterator it = ps.begin();it != ps.end();it++){
    algebra::VectorD<3> xyz1 = it->get_coordinates();
    for (XYZsTemp::const_iterator it1 = it+1;it1 != ps.end();it1++){
      algebra::VectorD<3> xyz2 = it1->get_coordinates();
      double curr_squared_dist = algebra::get_squared_distance(xyz1,xyz2);
      if (curr_squared_dist > max_squared_dist) {
        p1 = *it;
        p2 = *it1;
        max_squared_dist = curr_squared_dist;
      }
    }
  }
  return algebra::Segment3D(p1.get_coordinates(),
                            p2.get_coordinates());
}

IMPCORE_END_NAMESPACE

/**
 *  \file core/utility.cpp
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/core/utility.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
IMPCORE_BEGIN_NAMESPACE

algebra::Vector3D centroid(const Particles &ps) {
 algebra::Vector3D cen(0.0,0.0,0.0);
 for (Particles::const_iterator it = ps.begin(); it != ps.end();it++) {
   cen = cen + core::XYZ(*it).get_coordinates();
 }
 return cen/ps.size();
}

void transform(const Particles &ps,
               const algebra::Transformation3D &t) {
  algebra::Vector3D xyz_new;
  core::XYZ xyz_d;
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    if (core::XYZ::is_instance_of(*it)) {
      xyz_d = core::XYZ(*it);
      xyz_new = t.transform(xyz_d.get_coordinates());
      xyz_d.set_coordinates(xyz_new);
    }
  }
}

algebra::Segment3D diameter(const Particles &ps) {
  IMP_check(!ps.empty(), "Can't compute the diameter if you don't give "
            "me any particles.", ValueException);
  Particle *p1=NULL,*p2=NULL;
  double max_squared_dist=0.0;

  for (Particles::const_iterator it = ps.begin();it != ps.end();it++){
    algebra::Vector3D xyz1 = XYZ(*it).get_coordinates();
    for (Particles::const_iterator it1 = it+1;it1 != ps.end();it1++){
      algebra::Vector3D xyz2 = XYZ(*it1).get_coordinates();
      double curr_squared_dist = algebra::squared_distance(xyz1,xyz2);
      if (curr_squared_dist > max_squared_dist) {
        p1 = *it;
        p2 = *it1;
        max_squared_dist = curr_squared_dist;
      }
    }
  }
  return algebra::Segment3D(XYZ(p1).get_coordinates(),
                            XYZ(p2).get_coordinates());
}

IMPCORE_END_NAMESPACE

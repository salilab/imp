/**
 *  \file core/utility.cpp
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/core/utility.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZDecorator.h>
IMPCORE_BEGIN_NAMESPACE

IMPCOREEXPORT algebra::Vector3D
centroid(const Particles &ps) {
 algebra::Vector3D cen(0.0,0.0,0.0);
 for (Particles::const_iterator it = ps.begin(); it != ps.end();it++) {
   if (core::XYZDecorator::is_instance_of(*it)) {
     cen = cen + core::XYZDecorator(*it).get_coordinates();
   }
 }
 return cen*(1.0/ps.size());
}
IMPCOREEXPORT
void transform(const Particles &ps,
               const algebra::Transformation3D &t) {
  algebra::Vector3D xyz_new;
  core::XYZDecorator xyz_d;
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    if (core::XYZDecorator::is_instance_of(*it)) {
      xyz_d = core::XYZDecorator(*it);
      xyz_new = t.transform(xyz_d.get_coordinates());
      xyz_d.set_coordinates(xyz_new);
    }
  }
}
IMPCOREEXPORT
algebra::Segment3D diameter(const Particles &ps) {
  algebra::Vector3D lower_bound,upper_bound;
   //read the points and determine the dimentions of the map
  for (int i=0;i<3;i++) {
    lower_bound[i]=INT_MAX;
    upper_bound[i]=INT_MIN;
  }
  IMP::Float dist=INT_MIN;
  IMP::algebra::Vector3D xyz1;
  IMP::algebra::Vector3D xyz2;
  for (IMP::Particles::const_iterator it = ps.begin();it != ps.end();it++){
    if (core::XYZDecorator::is_instance_of(*it)){
      xyz1 = IMP::core::XYZDecorator(*it).get_coordinates();
      for (IMP::Particles::const_iterator it1 = it+1;it1 != ps.end();it1++){
        if (core::XYZDecorator::is_instance_of(*it1)){
          xyz2 = IMP::core::XYZDecorator(*it1).get_coordinates();
          if (IMP::algebra::distance(xyz1,xyz2) >dist) {
            lower_bound = xyz1;
            upper_bound = xyz2;
            dist = IMP::algebra::distance(xyz1,xyz2);
          }
        }
      }
    }
  }
  return algebra::Segment3D(lower_bound,upper_bound);
}

IMPCORE_END_NAMESPACE

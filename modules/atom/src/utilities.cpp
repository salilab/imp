/**
 *  \file atom/utilities.cpp
 *  \brief Functions to perform simple functions on proteins
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/atom/utilities.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZDecorator.h>
IMPATOM_BEGIN_NAMESPACE

IMPATOMEXPORT algebra::Vector3D
centroid(const MolecularHierarchyDecorator &m) {
 algebra::Vector3D centroid(0.0,0.0,0.0);
 Particles ps =
   get_by_type(m, MolecularHierarchyDecorator::ATOM);
 for (Particles::iterator it = ps.begin(); it != ps.end(); it++) {
   centroid = centroid + core::XYZDecorator::cast(*it).get_coordinates();
  }
 return centroid*(1.0/ps.size());
}
IMPATOMEXPORT
void transform(const MolecularHierarchyDecorator &m,
               const algebra::Transformation3D &t) {
  Particles ps =
    get_by_type(m, MolecularHierarchyDecorator::ATOM);
  algebra::Vector3D xyz_new;
  core::XYZDecorator xyz_d;
  for (Particles::iterator it = ps.begin(); it != ps.end(); it++) {
    xyz_d = core::XYZDecorator::cast(*it);
    xyz_new = t.transform(xyz_d.get_coordinates());
    xyz_d.set_coordinates(xyz_new);
  }
}
IMPATOMEXPORT
algebra::Line3D diameter(const MolecularHierarchyDecorator &m) {
  algebra::Vector3D lower_bound,upper_bound;
   //read the points and determine the dimentions of the map
  for (int i=0;i<3;i++) {
    lower_bound[i]=INT_MAX;
    upper_bound[i]=INT_MIN;
  }
  IMP::Float dist=INT_MIN;
 Particles ps =
   get_by_type(m, MolecularHierarchyDecorator::ATOM);
  for (IMP::Particles::const_iterator it = ps.begin();it != ps.end();it++){
    IMP::algebra::Vector3D xyz1 =
         IMP::core::XYZDecorator::cast(*it).get_coordinates();
    for (IMP::Particles::const_iterator it1 = it+1;it1 != ps.end();it1++){
      IMP::algebra::Vector3D xyz2 =
           IMP::core::XYZDecorator::cast(*it1).get_coordinates();
      if (IMP::algebra::distance(xyz1,xyz2) >dist) {
        lower_bound = xyz1;
        upper_bound = xyz2;
        dist = IMP::algebra::distance(xyz1,xyz2);
      }
    }
  }
  return algebra::Line3D(lower_bound,upper_bound);
}
IMPATOM_END_NAMESPACE

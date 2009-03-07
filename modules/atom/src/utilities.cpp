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
IMPATOM_END_NAMESPACE

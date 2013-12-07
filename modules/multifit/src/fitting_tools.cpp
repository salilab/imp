/**
 *  \file fitting_tools.cpp
 *  \brief tools for handling fitting records
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fitting_tools.h>
IMPMULTIFIT_BEGIN_NAMESPACE

FittingSolutionRecords get_close_to_point(
    const FittingSolutionRecords &fit_sols, atom::Hierarchy mh,
    IMP::kernel::Particle *ap, Float dist) {
  FittingSolutionRecords pruned_fit_sols;
  Float dist2 = dist * dist;
  IMP::algebra::Vector3D ap_centroid = IMP::core::XYZ(ap).get_coordinates();
  IMP::algebra::Vector3D mh_centroid =
      IMP::core::get_centroid(IMP::core::XYZs(IMP::core::get_leaves(mh)));
  for (FittingSolutionRecords::const_iterator it = fit_sols.begin();
       it != fit_sols.end(); it++) {
    if (IMP::algebra::get_squared_distance(
            it->get_fit_transformation().get_transformed(mh_centroid),
            ap_centroid) < dist2) {
      pruned_fit_sols.push_back(*it);
    }
  }
  return pruned_fit_sols;
}
IMPMULTIFIT_END_NAMESPACE

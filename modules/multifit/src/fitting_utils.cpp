/**
 *  \file fitting_utils.cpp
 *  \brief fitting utilities
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fitting_utils.h>
#include <IMP/algebra/vector_search.h>
IMPMULTIFIT_BEGIN_NAMESPACE

em::DensityMap *create_hit_map(core::RigidBody rb, Refiner *rb_ref,
                               const FittingSolutionRecords &sols,
                               em::DensityMap *dmap) {
  // get center
  algebra::Vector3D cen =
      core::get_centroid(core::XYZs(rb_ref->get_refined(rb)));
  // create a return map
  base::Pointer<em::DensityMap> ret(new em::DensityMap(*(dmap->get_header())));
  ret->reset_data(0.);
  algebra::Vector3D cen_t;
  // get all hits
  for (int i = 0; i < (int)sols.size(); i++) {
    cen_t = sols[i].get_fit_transformation().get_transformed(cen);
    if (ret->is_part_of_volume(cen_t)) {
      ret->set_value(cen_t[0], cen_t[1], cen_t[2], ret->get_value(cen_t) + 1);
    }
  }
  return ret.release();
}

namespace {
std::pair<Ints, Ints> seperate_points(const atom::Hierarchy &mh,
                                      const algebra::Vector3Ds points,
                                      Float max_dist) {
  Ints close_inds, far_inds;
  float max_dist2 = max_dist * max_dist;
  // index leaves of the molecule in a knn
  algebra::Vector3Ds all_cen;
  core::XYZs all_xyz(core::get_leaves(mh));
  for (int i = 0; i < (int)all_xyz.size(); i++) {
    all_cen.push_back(all_xyz[i].get_coordinates());
  }
  IMP_NEW(algebra::NearestNeighborD<3>, nn, (all_cen));
  for (unsigned int j = 0; j < points.size(); j++) {
    int closest_cen = nn->get_nearest_neighbor(points[j]);
    if (algebra::get_squared_distance(all_cen[closest_cen], points[j]) >
        max_dist2) {
      far_inds.push_back(j);
    } else {
      close_inds.push_back(j);
    }
  }
  return std::pair<Ints, Ints>(close_inds, far_inds);
}
}

algebra::Vector3Ds get_points_close_to_molecule(const atom::Hierarchy &mh,
                                                const algebra::Vector3Ds points,
                                                Float max_dist) {
  Ints close_inds = seperate_points(mh, points, max_dist).first;
  algebra::Vector3Ds ret;
  for (Ints::iterator it = close_inds.begin(); it != close_inds.end(); it++) {
    ret.push_back(points[*it]);
  }
  return ret;
}
algebra::Vector3Ds get_points_far_from_molecule(const atom::Hierarchy &mh,
                                                const algebra::Vector3Ds points,
                                                Float max_dist) {
  Ints far_inds = seperate_points(mh, points, max_dist).second;
  algebra::Vector3Ds ret;
  for (Ints::iterator it = far_inds.begin(); it != far_inds.end(); it++) {
    ret.push_back(points[*it]);
  }
  return ret;
}
IMPMULTIFIT_END_NAMESPACE

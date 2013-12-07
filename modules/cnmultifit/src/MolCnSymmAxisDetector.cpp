/**
 *  \file MolCnSymmAxisDetector.cpp
 *  \brief molecule symmetry detector
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/cnmultifit/MolCnSymmAxisDetector.h>
#include <algorithm>
#include <IMP/atom/distance.h>
#include <IMP/atom/pdb.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

MolCnSymmAxisDetector::MolCnSymmAxisDetector(int symm_deg,
                                             const atom::Hierarchies &mhs)
    : symm_deg_(symm_deg) {
  // get all vectors
  mhs_ = mhs;
  for (atom::Hierarchies::const_iterator it = mhs_.begin(); it != mhs_.end();
       it++) {
    core::XYZs temp_ps = core::XYZs(core::get_leaves(*it));
    for (unsigned int i = 0; i < temp_ps.size(); i++) {
      vecs_.push_back(temp_ps[i].get_coordinates());
    }
  }
  // calculate  pca
  pca_ = algebra::get_principal_components(vecs_);
  // calculate transformation from the native axes system to the
  // one defined by the pca
  from_native_ = algebra::get_rotation_from_x_y_axes(
      pca_.get_principal_component(0), pca_.get_principal_component(1));
  to_native_ = from_native_.get_inverse();
}

int MolCnSymmAxisDetector::get_symmetry_axis_index() const {
  int symm_ind = 0;
  float symm_s, tmp_s;
  symm_s = calc_symm_score(0);
  tmp_s = calc_symm_score(1);
  if (tmp_s < symm_s) {
    symm_s = tmp_s;
    symm_ind = 1;
  }
  tmp_s = calc_symm_score(2);
  if (tmp_s < symm_s) {
    symm_s = tmp_s;
    symm_ind = 2;
  }
  return symm_ind;
}
algebra::Vector3D MolCnSymmAxisDetector::get_symmetry_axis() const {
  return pca_.get_principal_component(get_symmetry_axis_index());
}

// only rotation
// axis_symm will be used as z-axis and the center of mass is assumed to be 000
float MolCnSymmAxisDetector::calc_symm_score(int symm_axis_ind) const {
  int ind1, ind2;
  if (symm_axis_ind == 0) {
    ind1 = 1;
    ind2 = 2;
  } else if (symm_axis_ind == 1) {
    ind1 = 0;
    ind2 = 2;
  } else {
    ind1 = 0;
    ind2 = 1;
  }
  // symm axis is in the PCA reference frame.
  // we want to move it to the native resference frame

  float angle = 2 * PI / symm_deg_;
  algebra::Transformation3D from_native_xy_to_pca_xy =
      algebra::get_rotation_from_x_y_axes(pca_.get_principal_component(ind1),
                                          pca_.get_principal_component(ind2));
  // translate the PCA origin to 000
  algebra::Transformation3D pca_to_native_translation =
      algebra::Transformation3D(algebra::get_identity_rotation_3d(),
                                -pca_.get_centroid());
  // build a transformation such that the symm axis is on Z
  algebra::Transformation3D from_pca_to_native =
      from_native_xy_to_pca_xy.get_inverse() * pca_to_native_translation;
  // build the symmetric transformation (move to native, rotate around Z
  //, and move back to PCA reference frame)
  algebra::Transformation3D symmetric_trans =
      from_pca_to_native.get_inverse() *
      algebra::Transformation3D(algebra::get_rotation_about_axis(
                                    algebra::Vector3D(0., 0., 1.), angle),
                                algebra::Vector3D(0., 0., 0.)) *
      from_pca_to_native;
  float rmsd2 = 0;
  float rmsd3 = 0.;
  atom::Hierarchy mh1 = mhs_[0];
  core::XYZs mh1_xyz = core::XYZs(core::get_leaves(mh1));
  atom::Hierarchy mh2 = mhs_[mhs_.size() - 1];
  core::XYZs mh2_xyz = core::XYZs(core::get_leaves(mh2));
  atom::Hierarchy mh3 = mhs_[1];
  core::XYZs mh3_xyz = core::XYZs(core::get_leaves(mh3));
  algebra::Vector3D loc, loc2, loc3, symm_loc;
  // we use both mh2 and mh3 because sometimes the input
  // is clock wise and sometimes
  // counter clock wise
  for (unsigned int i = 0; i < mh1_xyz.size(); i++) {
    loc = mh1_xyz[i].get_coordinates();
    symm_loc = symmetric_trans.get_transformed(loc);
    loc2 = mh2_xyz[i].get_coordinates();
    rmsd2 += algebra::get_squared_distance(symm_loc, loc2);
    loc3 = mh3_xyz[i].get_coordinates();
    rmsd3 += algebra::get_squared_distance(symm_loc, loc3);
  }
  return std::min(std::sqrt(rmsd2 / mh1_xyz.size()),
                  std::sqrt(rmsd3 / mh1_xyz.size()));
}

IMPCNMULTIFIT_END_NAMESPACE

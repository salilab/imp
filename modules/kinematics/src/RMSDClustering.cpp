/**
 * \file IMP/kinematics/RMSDClustering.cpp
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include  <IMP/kinematics/RMSDClustering.h>

IMPKINEMATICS_BEGIN_NAMESPACE

unsigned int rmsd_clustering(const std::vector<IMP::algebra::Vector3Ds>& coords_vec,
                             std::vector<int>& out_cluster_numbers,
                             float rmsd_thr, bool compute_trans) {

  out_cluster_numbers.resize(coords_vec.size(), -1); // init cluster numbers

  unsigned int cluster_counter = 0;
  for (unsigned int i = 0; i < out_cluster_numbers.size(); i++) {
    if (out_cluster_numbers[i] == -1) { // new cluster
      out_cluster_numbers[i] = cluster_counter;
      // group others into this cluster
      for (unsigned int j = i+1; j < out_cluster_numbers.size(); j++) {
        // calculate RMSD
        double rmsd = 0;
        if (compute_trans) {
          IMP::algebra::Transformation3D tr =
            IMP::algebra::get_transformation_aligning_first_to_second(coords_vec[i],
                                                                      coords_vec[j]);
          rmsd = IMP::algebra::get_rmsd_transforming_first(tr,
                                                           coords_vec[i],
                                                           coords_vec[j]);
        } else {
          rmsd = IMP::algebra::get_rmsd(coords_vec[i], coords_vec[j]);
        }
        if (rmsd <= rmsd_thr) out_cluster_numbers[j] = cluster_counter;
      }
      cluster_counter++;
    }
    if(i%100 == 0) {
      std::cerr << "clustering structure " << i << " # clusters " << cluster_counter << std::endl;
    }
  }
  return cluster_counter;
}

IMPKINEMATICS_END_NAMESPACE

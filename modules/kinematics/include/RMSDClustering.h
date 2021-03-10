/**
 * \file IMP/kinematics/RMSDClustering.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_RMSD_CLUSTERING_H
#define IMPKINEMATICS_RMSD_CLUSTERING_H

#include <IMP/kinematics/kinematics_config.h>

#include <IMP/algebra/geometric_alignment.h>
#include <IMP/algebra/distance.h>

#include <vector>

IMPKINEMATICS_BEGIN_NAMESPACE

IMPKINEMATICSEXPORT
unsigned int rmsd_clustering(const std::vector<IMP::algebra::Vector3Ds>& coords_vec,
                             std::vector<int>& out_cluster_numbers,
                             float rmsd_thr, bool compute_trans);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_RMSD_CLUSTERING_H */

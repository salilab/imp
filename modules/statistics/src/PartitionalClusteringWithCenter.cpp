/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/PartitionalClusteringWithCenter.h>
#include <IMP/base/check_macros.h>

IMPSTATISTICS_BEGIN_NAMESPACE

unsigned int PartitionalClusteringWithCenter::get_number_of_clusters() const {
  IMP_CHECK_OBJECT(this);
  return clusters_.size();
}
const Ints &PartitionalClusteringWithCenter::get_cluster(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(), "There are only "
                                                    << get_number_of_clusters()
                                                    << " clusters. Not " << i);
  return clusters_[i];
}
int PartitionalClusteringWithCenter::get_cluster_representative(unsigned int i)
    const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(), "There are only "
                                                    << get_number_of_clusters()
                                                    << " clusters. Not " << i);
  return reps_[i];
}
IMPSTATISTICS_END_NAMESPACE

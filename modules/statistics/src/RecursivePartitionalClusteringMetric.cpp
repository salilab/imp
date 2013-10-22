/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/RecursivePartitionalClusteringMetric.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
IMPSTATISTICS_BEGIN_NAMESPACE
RecursivePartitionalClusteringMetric::RecursivePartitionalClusteringMetric(
    Metric *metric, PartitionalClustering *clustering)
    : Metric("RecursivePartitionalClusteringMetric%1%"),
      metric_(metric),
      clustering_(clustering) {}

double RecursivePartitionalClusteringMetric::get_distance(
    unsigned int i, unsigned int j) const {
  return metric_->get_distance(clustering_->get_cluster_representative(i),
                               clustering_->get_cluster_representative(j));
}

unsigned int RecursivePartitionalClusteringMetric::get_number_of_items() const {
  return clustering_->get_number_of_clusters();
}

PartitionalClustering *
RecursivePartitionalClusteringMetric::create_full_clustering(
    PartitionalClustering *center_cluster) {
  IMP::base::Vector<Ints> clusters(center_cluster->get_number_of_clusters());
  Ints reps(clusters.size());
  for (unsigned int i = 0; i < clusters.size(); ++i) {
    Ints outer = center_cluster->get_cluster(i);
    reps[i] = clustering_->get_cluster_representative(
        center_cluster->get_cluster_representative(i));
    for (unsigned int j = 0; j < outer.size(); ++j) {
      Ints inner = clustering_->get_cluster(outer[j]);
      clusters[i].insert(clusters[i].end(), inner.begin(), inner.end());
    }
  }
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters, reps));
  validate_partitional_clustering(ret, metric_->get_number_of_items());
  return ret.release();
}

IMPSTATISTICS_END_NAMESPACE

/**
 *  \file IMP/statistics/RecursivePartitionalClusteringMetric.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_RECURSIVE_PARTITIONAL_CLUSTERING_METRIC_H
#define IMPSTATISTICS_RECURSIVE_PARTITIONAL_CLUSTERING_METRIC_H

#include <IMP/statistics/statistics_config.h>
#include "metric_macros.h"
#include "Metric.h"
#include "PartitionalClustering.h"

IMPSTATISTICS_BEGIN_NAMESPACE

/** Represent a metric for clustering data that has already been clustered
    once. To use it, cluster things once, create one of these with the metric
    you want (created with the original data). When you pass this metric to
    the clustering algorithm, it will cluster the centers. You can extract the
    clustering of the original elements using create_full_clustering().
*/
class IMPSTATISTICSEXPORT RecursivePartitionalClusteringMetric : public Metric {
  IMP::OwnerPointer<Metric> metric_;
  IMP::OwnerPointer<PartitionalClustering> clustering_;

 public:
  RecursivePartitionalClusteringMetric(Metric *metric,
                                       PartitionalClustering *clustering);
  PartitionalClustering *create_full_clustering(
      PartitionalClustering *center_cluster);
  IMP_METRIC(RecursivePartitionalClusteringMetric);
};

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_RECURSIVE_PARTITIONAL_CLUSTERING_METRIC_H */

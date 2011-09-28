/**
 *  \file metric_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSTATISTICS_METRIC_CLUSTERING_H
#define IMPSTATISTICS_METRIC_CLUSTERING_H

#include "statistics_config.h"
#include "statistics_macros.h"
#include "PartitionalClustering.h"
#include "point_clustering.h"
#include <IMP/algebra/VectorD.h>
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/OwnerPointer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Compute a distance between two elements to be clustered
/** Metric clustering needs a way of computing the
    distances between the things being clustered.
 */
class IMPSTATISTICSEXPORT Metric: public IMP::base::Object {
public:
  Metric(std::string name);
  virtual double get_distance(unsigned int i,
                              unsigned int j) const =0;
  virtual unsigned int get_number_of_items() const=0;
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Metric);
};

IMP_OBJECTS(Metric, Metrics);

class IMPSTATISTICSEXPORT EuclideanMetric: public Metric {
  IMP::OwnerPointer<Embedding> em_;
public:
  EuclideanMetric(Embedding *em);
  IMP_METRIC(EuclideanMetric);
};


/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSTATISTICSEXPORT ConfigurationSetRMSDMetric: public Metric {
  IMP::OwnerPointer<ConfigurationSet> cs_;
  IMP::OwnerPointer<SingletonContainer> sc_;
  bool align_;
 public:
  ConfigurationSetRMSDMetric(ConfigurationSet *cs,
                              SingletonContainer *sc,
                              bool align=false);
  IMP_METRIC(ConfigurationSetRMSDMetric);
};



/** Represent a metric for clustering data that has already been clustered
    once. To use it, cluster things once, create one of these with the metric
    you want (created with the original data). When you pass this metric to
    the clustering algorithm, it will cluster the centers. You can extract the
    clustering of the original elements using create_full_clustering().
*/
class IMPSTATISTICSEXPORT RecursivePartitionalClusteringMetric: public Metric {
  IMP::OwnerPointer<Metric> metric_;
  IMP::OwnerPointer<PartitionalClustering> clustering_;
 public:
  RecursivePartitionalClusteringMetric(Metric *metric,
                                       PartitionalClustering *clustering);
  PartitionalClustering*
    create_full_clustering(PartitionalClustering *center_cluster);
  IMP_METRIC(RecursivePartitionalClusteringMetric);
};


/** Cluster by repeatedly removing edges which have lots
    of shortest paths passing through them. The process is
    terminated when there are a set number of
    connected components. Other termination criteria
    can be added if someone proposes them.

    Only items closer than far are connected.
 */
IMPSTATISTICSEXPORT
PartitionalClustering *create_centrality_clustering(Metric *d,
                                                 double far,
                                                 int k);


/** Cluster the elements into clusters with at most the specified
    diameter.
 */
IMPSTATISTICSEXPORT
PartitionalClustering *create_diameter_clustering(Metric *d,
                                                  double maximum_diameter);

/**Two points, \f$p_i\f$, \f$p_j\f$ are in the same cluster if
    there is a sequence of points \f$\left(p^{ij}_{0}\dots p^{ij}_k\right)\f$
    such that \f$\forall l ||p^{ij}_l-p^{ij}_{l+1}|| < d\f$.
 */
IMPSTATISTICSEXPORT PartitionalClustering*
create_connectivity_clustering(Metric *metric,
                               double dist);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRIC_CLUSTERING_H */

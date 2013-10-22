/**
 *  \file IMP/statistics/metric_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_METRIC_CLUSTERING_H
#define IMPSTATISTICS_METRIC_CLUSTERING_H

#include <IMP/statistics/statistics_config.h>
#include "PartitionalClustering.h"
#include "Metric.h"
#include "point_clustering.h"
#include <IMP/algebra/VectorD.h>
#include <IMP/macros.h>
#include <IMP/base/Object.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/SingletonContainer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

/** Cluster by repeatedly removing edges which have lots
    of shortest paths passing through them. The process is
    terminated when there are a set number of
    connected components. Other termination criteria
    can be added if someone proposes them.

    Only items closer than far are connected.
 */
IMPSTATISTICSEXPORT PartitionalClustering *create_centrality_clustering(
    Metric *d, double far, int k);

/** Cluster the elements into clusters with at most the specified
    diameter.
 */
IMPSTATISTICSEXPORT PartitionalClustering *create_diameter_clustering(
    Metric *d, double maximum_diameter);

/**Two points, \f$p_i\f$, \f$p_j\f$ are in the same cluster if
    there is a sequence of points \f$\left(p^{ij}_{0}\dots p^{ij}_k\right)\f$
    such that \f$\forall l ||p^{ij}_l-p^{ij}_{l+1}|| < d\f$.
 */
IMPSTATISTICSEXPORT PartitionalClustering *create_connectivity_clustering(
    Metric *metric, double dist);

/** Cutoff-based clustering as defined in
    Daura et al. Angew. Chem. Int. Ed. 1999. 38(1‐2): p. 236-240.
 */
IMPSTATISTICSEXPORT PartitionalClustering *create_gromos_clustering(
    Metric *d, double cutoff);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_METRIC_CLUSTERING_H */

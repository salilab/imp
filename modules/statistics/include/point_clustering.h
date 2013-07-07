/**
 *  \file IMP/statistics/point_clustering.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_POINT_CLUSTERING_H
#define IMPSTATISTICS_POINT_CLUSTERING_H

#include <IMP/statistics/statistics_config.h>
#include "GaussianMixtureModel.h"
#include "PartitionalClusteringWithCenter.h"
#include "Embedding.h"

IMPSTATISTICS_BEGIN_NAMESPACE

/** Return a k-means clustering of all points contained in the
    embedding (ie [0... embedding->get_number_of_embeddings())).
    These points are then clustered into k clusters. More iterations
    takes longer but produces a better clustering.

    The algorithm uses algebra::EuclideanVectorKDMetric for computing
    distances between embeddings and cluster centers. This can be
    parameterized if desired.
*/
IMPSTATISTICSEXPORT PartitionalClusteringWithCenter *create_lloyds_kmeans(
    Embedding *embedding, unsigned int k, unsigned int iterations);

/** Two points, \f$p_i\f$, \f$p_j\f$ are in the same cluster if
    there is a sequence of points \f$\left(p^{ij}_{0}\dots p^{ij}_k\right)\f$
    such that \f$\forall l ||p^{ij}_l-p^{ij}_{l+1}|| < d\f$.
 */
IMPSTATISTICSEXPORT PartitionalClusteringWithCenter *
    create_connectivity_clustering(Embedding *embed, double dist);

/** The space is grided with bins of side size and all points
    that fall in the same grid bin are made part of the same cluster.
*/
IMPSTATISTICSEXPORT PartitionalClusteringWithCenter *
    create_bin_based_clustering(Embedding *embed, double side);

/** Cluster by repeatedly removing edges which have lots
    of shortest paths passing through them. The process is
    terminated when there are a set number of
    connected components. Other termination criteria
    can be added if someone proposes them.
 */
IMPSTATISTICSEXPORT PartitionalClustering *create_centrality_clustering(
    Embedding *d, double far, int k);

/** Create Gaussian mixture model from some points
 */
IMPSTATISTICSEXPORT GaussianMixtureModel
    create_gaussian_mixture_model(Embedding *d, int k);


IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_POINT_CLUSTERING_H */

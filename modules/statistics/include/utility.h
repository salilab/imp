/**
 *  \file IMP/statistics/utility.h
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_UTILITY_H
#define IMPSTATISTICS_UTILITY_H

#include <IMP/statistics/statistics_config.h>
#include "PartitionalClustering.h"
#include "Embedding.h"

IMPSTATISTICS_BEGIN_NAMESPACE

/** Given a clustering and an embedding, compute the centroid for each cluster
 */
IMPSTATISTICSEXPORT algebra::VectorKDs get_centroids(Embedding* d,
                                                     PartitionalClustering* pc);

/** Given a clustering and an embedding, compute a representatative
    element for each cluster.
 */
IMPSTATISTICSEXPORT Ints get_representatives(Embedding* d,
                                             PartitionalClustering* pc);

IMPSTATISTICS_END_NAMESPACE

#endif /* IMPSTATISTICS_UTILITY_H */

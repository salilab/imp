/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/utility.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/base/object_macros.h>
IMPSTATISTICS_BEGIN_NAMESPACE

algebra::VectorKDs get_centroids(Embedding* d, PartitionalClustering* pc) {
  base::Pointer<Embedding> pd(d);
  base::Pointer<PartitionalClustering> ppc(pc);
  algebra::VectorKDs ret(pc->get_number_of_clusters());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    algebra::VectorKD cur =
        algebra::get_zero_vector_kd(d->get_point(0).get_dimension());
    Ints cc = pc->get_cluster(i);
    for (unsigned int j = 0; j < cc.size(); ++j) {
      cur += d->get_point(cc[j]);
    }
    cur /= cc.size();
    ret[i] = cur;
  }
  return ret;
}

Ints get_representatives(Embedding* d, PartitionalClustering* pc) {
  base::Pointer<Embedding> pd(d);
  base::Pointer<PartitionalClustering> ppc(pc);
  algebra::VectorKDs centroids = get_centroids(d, pc);
  Ints ret(centroids.size());
  IMP_NEW(algebra::NearestNeighborKD, nn, (d->get_points()));
  for (unsigned int i = 0; i < centroids.size(); ++i) {
    ret[i] = nn->get_nearest_neighbors(centroids[i], 1)[0];
  }
  return ret;
}

IMPSTATISTICS_END_NAMESPACE

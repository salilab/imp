/**
 *  \file clustering_helper.h
 *  \brief Implementation details for clustering algorithms
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_INTERNAL_CLUSTERING_HELPER_H
#define IMPEM2D_INTERNAL_CLUSTERING_HELPER_H

#include "IMP/em2d/em2d_config.h"

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

typedef std::pair<unsigned int, double> pair_cluster_id_distance;
typedef std::list<pair_cluster_id_distance> list_cluster_id_distance;

class ListHasDistance {
  unsigned l1_, l2_;
public:
  ListHasDistance(unsigned l1, unsigned l2) : l1_(l1), l2_(l2) {}
  bool operator()(const pair_cluster_id_distance& cid) {
    return cid.first == l1_ || cid.first == l2_;
  }
};

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_CLUSTERING_HELPER_H */

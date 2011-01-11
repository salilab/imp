/**
 *  \file TransformationsCluster.cpp
 *  \brief TransformationsCluster implementation
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/TransformationsCluster.h"

IMPEM2D_BEGIN_NAMESPACE

void TransformationsCluster::add_transformation(
                                   const TransformationsCluster &cluster) {
  all_transformations.push_back(cluster.get_representative_transformation());
  // If the representative transformation of the argument is better than
  // the current one, adopt it.
  if(cluster.get_score() >  get_score()) {
    representative_em2d_score_ = cluster.get_score();
    representative_trans_ = cluster.get_representative_transformation();
  }
}

IMPEM2D_END_NAMESPACE

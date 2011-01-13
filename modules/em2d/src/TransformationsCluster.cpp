/**
 *  \file TransformationsCluster.cpp
 *  \brief TransformationsCluster implementation
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/TransformationsCluster.h"

IMPEM2D_BEGIN_NAMESPACE

void TransformationsCluster::join_into(
                                   const TransformationsCluster &cluster) {
  // Transfer all transformations to the cluster
  // If the representative transformation of the argument is better than
  // the current one, adopt it.
  if(cluster.get_score() >  get_score()) {
    representative_score_ = cluster.get_score();
    representative_trans_ = cluster.get_representative_transformation();
    representative_model_id_ = cluster.get_representative_model_id();
  }
  for (unsigned int i=0;i<cluster.get_number_of_members();++i) {
    all_transformations_.push_back(cluster.get_individual_transformation(i));
    all_models_ids_.push_back(cluster.get_individual_model_id(i));
    all_scores_.push_back(cluster.get_individual_score(i));
  }
}

IMPEM2D_END_NAMESPACE

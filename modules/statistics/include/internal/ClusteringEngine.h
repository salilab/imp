/**
 *  \file IMP/statistics/ClusteringEngine.h
 *  \brief Virtual anchor points clustering engine
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_CLUSTERING_ENGINE_H
#define IMPSTATISTICS_INTERNAL_CLUSTERING_ENGINE_H

#include <IMP/statistics/statistics_config.h>
#include "DataPoints.h"

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

class ClusteringEngine {
public:
  virtual ~ClusteringEngine(){}
  virtual bool is_part_of_cluster(int data_point_ind,int cluster_ind) const=0;
  // TODO: convert Array1DD to standard IMP base types?
  virtual Array1DD get_center(int center_ind) const=0;
  virtual int get_number_of_clusters() const=0;
};
IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_CLUSTERING_ENGINE_H */

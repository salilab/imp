/**
 *  \file ClusteringEngine.h
 *  \brief Virtual anchor points clustering engine
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_CLUSTERING_ENGINE_H
#define IMPMULTIFIT_CLUSTERING_ENGINE_H

#include "DataPoints.h"
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE

class ClusteringEngine {
public:
  virtual ~ClusteringEngine(){}
  virtual bool is_part_of_cluster(int data_point_ind,int cluster_ind) const{
    return true;}
  virtual Array1DD get_center(int center_ind) const{return Array1DD();}
  virtual int get_number_of_clusters() const{return 0;}
};
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_CLUSTERING_ENGINE_H */

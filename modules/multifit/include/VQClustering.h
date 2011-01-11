/**
 *  \file VQClustering.h
 *  \brief Vector quantization clustering. Based on Wriggers et at, JMB 1998
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_VQ_CLUSTERING_H
#define IMPMULTIFIT_VQ_CLUSTERING_H

#include <IMP/base_types.h>
#include "DataPoints.h"
#include "ClusteringEngine.h"
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE
class IMPMULTIFITEXPORT VQClustering : public ClusteringEngine {
public:
  VQClustering();
  VQClustering(DataPoints *data, int k);
  ~VQClustering() {}

  void run(DataPoints *starting_centers=NULL);

  bool is_part_of_cluster(int data_point_ind,int cluster_ind) const{
    return (get_cluster_assignment(data_point_ind) == cluster_ind);
  }

  int get_cluster_assignment(int data_point_ind) const {
    IMP_USAGE_CHECK(is_set_,
                    " the clusters have not been assigned " << std::endl);
    IMP_USAGE_CHECK((unsigned int)data_point_ind<assignment_.size(),
                    " no cluster has been assigned to data point "
                    <<  data_point_ind << std::endl);
    return assignment_[data_point_ind];
  }

  Array1DD get_center(int center_ind) const {
    IMP_USAGE_CHECK(is_set_,
                    " the clusters have not been assigned " << std::endl);
    return centers_[center_ind];
  }
  int get_number_of_clusters() const {return k_;}
  void set_random_offset(double o){random_offset_=o;}
  const DataPoints *get_full_data() const {return full_data_;}
  void set_fast_clustering();
protected:
  /**
  Sample possible centers.
  The function is divided into runs and each run into steps.
  At each run, centers are randomly selected from the data points.
  At each stage, the attributes of the selecteds center are then refined
  to best match a randmoly selected data point.
  The centers are updated in the following way:
  1. The centers are first sorted from the closest to the
  farthest to the randomly selected data point
  2. ci(t)=ci(t-1)+epsilon(t)*eps(-ki(t)/l(t))[v(t)-ci(t-1)]
      epsilon(t=step_ind) =
      ei_ * exp((1.*step_ind/number_of_steps_)*log(ef_/ei_));
      lambda(t=step_ind) =
      li_ * exp((1.*step_ind/number_of_steps_)*log(lf_/li_));
      ki(t) -
      measure the effect of data point v(t) on the center.
      The effect is proportional
      to the distance of the point from the center.
  \param[in] tracking keeps all of the sampled centers
  throughout the sampling procedure
  \note see Wriggers et at, JMB 1998
 */
  void sampling(Array1DD_VEC *tracking);
//! Try to improve the centers of the clusters according
//! to the sampled centers
/**
\param[in] tracking all sampled centers
\param[in] centers  the final centers
 */
  void clustering(Array1DD_VEC *tracking, Array1DD_VEC *centers);
  void set_assignments();
  //void set_centers_as_particles();
  void sample_data_point(Array1DD &p);
  //sample initial centers from the data
  void center_sampling( Array1DD_VEC *centers_sample);
  int dim_;
  int k_;//number of centers
  bool is_set_; //is_set_ is true if the clustering was preformed
  const Array1DD_VEC * data_;
  DataPoints *full_data_;
  //exe parameters
  int number_of_runs_;
  int number_of_steps_;
  double  ei_,ef_;// parameters for epsilon updates
  double  li_,lf_;// parameters for lamda updates
  double random_offset_; //random offset for point sampling
  Array1DD_VEC centers_;
  std::vector<int> assignment_;//the assignment of data points to clusters
};
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_VQ_CLUSTERING_H */

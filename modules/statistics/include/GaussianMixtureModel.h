/**
 *  \file IMP/statistics/GaussianMixtureModel.h
 *  \brief GMM implementation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_GAUSSIAN_MIXURE_MODEL_H
#define IMPSTATISTICS_GAUSSIAN_MIXURE_MODEL_H

#include <IMP/statistics/statistics_config.h>
#include <IMP/statistics/Embedding.h>
#include "GaussianComponent.h"
#include "PartitionalClusteringWithCenter.h"
#include "internal/DataPoints.h"
#include "internal/ClusteringEngine.h"
#include <IMP/base_types.h>
#include <IMP/Particle.h>


IMPSTATISTICS_BEGIN_NAMESPACE

class IMPSTATISTICSEXPORT GaussianMixtureModel :
                 public internal::ClusteringEngine {
 public :
  GaussianMixtureModel(){}
  /**
  /param[in] dim  the size of the data to be added
  /param[in] k    the number of Gaussians to use
   */
  GaussianMixtureModel(int dim,int k);
  //here you need to get particles and transform it to data_ and data_r_
  /**
  /param[in] data the data to model using the GMM
  /param[in] k    the number of Gaussians to use
   */
  GaussianMixtureModel(statistics::internal::DataPoints *data,int k);
  /**
  /param[in] data the data to model using the GMM
  /param[in] k the number of Gaussians to use
  /param[in] guesses initial ellipsoidal guesses for the Gaussians
   */
  GaussianMixtureModel(statistics::internal::DataPoints *data,int k,
                       algebra::Ellipsoid3Ds guesses);
  /**
  /param[in] e Embedding from which to extract data
  /param[in] init_clusters Initial clustering
   */
  GaussianMixtureModel(Embedding * e,
                       PartitionalClusteringWithCenter * init_clusters);

  ~GaussianMixtureModel() {}

 GaussianComponent * get_gaussian_component(int i) {
    return components_[i];
  }
  void set_gaussian_component(GaussianComponent * g,int i) {
    components_[i]=g;
    //todo - set invalid
  }
  unsigned int get_num_steps() const {
    return num_steps_;
  }
  void set_num_steps(unsigned int num_steps) {
    num_steps_=num_steps;
  }
  //! Run Expectation-Maximization
  /**
  \param[in] num_steps the number of EM steps to run
   */
  void run();
  void show(std::ostream &out=std::cout) const;
  //! Compute the expected values of the responsibilites of data points
  //!(how much they belong to each cluster)
  /**
  The new probability of data point i to belong to cluster k:
  data_r_[i,k] = p_k*N(data_[i]|mean_k,sigma_k)
                 --------------------------------
                 sum_over_k(p_k*N(data_[i]|mean_k,sigma_k))

   p_k - is the prior of cluster k
   */
  virtual void e_step();
  const Array2DD * get_responsibilites() const {return &data_r_;}
  double get_log_likelihood() const {return log_lik_;}
  //! Get the most probable cluster for a data point
  int get_cluster_assignment(int data_point_ind) const {
    int ci=0;
    for(unsigned int i=0;i<k_;i++) {
      if (data_r_[data_point_ind][ci]<data_r_[data_point_ind][i]) {
        ci=i;
      }
    }
    return ci;
  }

  bool is_part_of_cluster(int data_point_ind,int cluster_ind) const{
    //TODO - the threshold should be a parameter
    //TODO - use the commented line, I am not using it now
    //   because a particle can not be a part of more then one rigid body.
    //return (get_cluster_probability(data_point_ind,cluster_ind) >0.3);
    return (cluster_ind==get_cluster_assignment(data_point_ind));

  }

  double get_cluster_probability(int data_point_ind,int cluster_ind) const {
    return data_r_[data_point_ind][cluster_ind];
  }
  int get_number_of_clusters() const {return k_;}
  Array1DD get_center(int ci) const {return components_[ci]->get_mean();}
  void set_center(int ci,const Array1DD &c) {components_[ci]->set_mean(c);}
  Array1DD get_center() const;
  Array2DD get_sigma(int ci) const {return components_[ci]->get_sigma();}
  //! Get the sigma of the data points in the GMM
  Array2DD get_data_sigma() const;
  double get_prior(int ci) const {return components_[ci]->get_prior();}
  void set_prior(int ci,double p) {components_[ci]->set_prior(p);}
  const statistics::internal::DataPoints *get_full_data() const {
    return full_data_;}
  //! Change the data the GMM discribes. Gaussian components remain unchanged
  //! The dara_r_ and data_p_ are reset
  void change_data(statistics::internal::DataPoints *data);
  //calculate how much the data point is part of the pdf
  //TODO - maybe we need to change
  double pdf(int data_point_ind) {
    double val=0.;
    for(unsigned int i=0;i<k_;i++) {
      val+=get_prior(i)*components_[i]->pdf((*data_)[data_point_ind]);
    }
    return val;
  }
protected:

  //! Update the components parameters acoording to the new responsibilites
  /**
  The new probability of data point i to belong to cluster k:

  prior_k = sum_over_i {data_r_[i][k]}              (scalar)
            --------------------------
                number_of_data_points

  mean_k =  sum_over_i {data_r_[i][k]*data_[i]}     (vector of size dim_)
            -----------------------------------
                 sum_over_i {data_r_[i][k]}

  sigma_k = sum_over_i {data_r_[i][k]*dist(data_[i],mean_k)}
            ------------------------------------------------
                     sum_over_i {data_r_[i][k]}

    (matrix of size dim_*dim_)

  All values of in the sigma matrix of component k have the same value : sigma_k
   */
  virtual void m_step();
  //! Setup the initial Gaussian components.
  //! The data points are divided to k clusters
  //!     according to the value at the first dimension.
  //! The mean and sigma are calculated according to these clusters.
  void setup();
  void update_log_likelihood();
  //  void log_intermediates(int step_ind);
  unsigned int data_n_; // number of data points
  unsigned int k_; //the number of clusters
  unsigned int dim_; //the dimension of the data points
  unsigned int num_steps_;
  double log_lik_;
  const std::vector<Array1DD> *data_;
  const std::vector<IMP::Ints >*data_log_; //keep for each optimization step
          //   the best component for each data point
  statistics::internal::DataPoints* full_data_;
  Array2DD data_r_; //responsibilities of data points (N*K)
  Array2DD data_p_; // the probability of each data point
       //to belong to each of the clusters
  std::vector<base::Pointer<GaussianComponent> > components_;
  friend class HierarchicalGaussianMixtureModel;
  static const double GMM_EPS=0.00000000000000000000001; //TODO - change?
};
IMP_VALUES(GaussianMixtureModel, GaussianMixtureModels);

IMPSTATISTICS_END_NAMESPACE
#endif /* IMPSTATISTICS_GAUSSIAN_MIXURE_MODEL_H */

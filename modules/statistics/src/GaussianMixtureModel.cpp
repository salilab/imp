/**
 *  \file GaussianMixtureModel.cpp
 *  \brief GMM implementation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/GaussianMixtureModel.h>
#include <IMP/constants.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <limits.h>
#include <boost/timer.hpp>
#include <boost/progress.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE
GaussianMixtureModel::GaussianMixtureModel(int dim,int k) {
  dim_ = (*data_)[0].dim1();
  k_=k;
  for(unsigned int k=0;k<k_;k++) {
    components_.push_back(new GaussianComponent(Array1DD(dim_,0.),
                                                Array2DD (dim_,dim_,0.),0.,k));
  }
  num_steps_=300;
}

GaussianMixtureModel::GaussianMixtureModel(
                     statistics::internal::DataPoints *data,int k) {
  full_data_ = data;
  data_=full_data_->get_data();
  dim_ = (*data_)[0].dim1();
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"number of data points  : " <<
            full_data_->get_number_of_data_points() << std::endl);
    IMP_LOG(VERBOSE,"data dimension  : " << dim_ << std::endl);
  }
  k_=k;
  data_n_ = data_->size();
  data_r_ = Array2DD(data_n_,k_,1./k_);
  num_steps_=300;
  //data_p_ = Array2DD(data_n_,k_);
  setup();
}

GaussianMixtureModel::GaussianMixtureModel(
                statistics::internal::DataPoints *data,int k,
                algebra::Ellipsoid3Ds guesses) {
  full_data_ = data;
  data_=full_data_->get_data();
  dim_ = (*data_)[0].dim1();
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"number of data points  : " <<
            full_data_->get_number_of_data_points() << std::endl);
    IMP_LOG(VERBOSE,"data dimension  : " << dim_ << std::endl);
  }
  k_=k;
  data_n_ = data_->size();
  data_r_ = Array2DD(data_n_,k_,1./k_);
  num_steps_=300;
  //data_p_ = Array2DD(data_n_,k_);
  for(unsigned int k=0;k<k_;k++) {
    components_.push_back(new GaussianComponent(guesses[k],1./k_,k));
  }
}

GaussianMixtureModel::GaussianMixtureModel(Embedding * e,
                            PartitionalClusteringWithCenter * init_clusters){
  std::cout<<"Hello world"<<std::endl;
}

Array1DD GaussianMixtureModel::get_center() const {
  Array1DD m(dim_,0.);
  for(unsigned int i=0;i<k_;i++) {
    m = algebra::internal::TNT::add(m,components_[i]->get_mean());
  }
  return algebra::internal::TNT::multiply(1./k_,m);
}

void GaussianMixtureModel::setup() {
  //calculate the min/max value for the first attribute
  double max_val=-1.*INT_MAX;
  double min_val=1.*INT_MAX;
  IMP_LOG(VERBOSE,"GMM setup "<<std::endl);

  for(unsigned int i=0;i<data_n_;i++) {
    if ((*data_)[i][0] > max_val) {
      max_val = (*data_)[i][0];
    }
    if ((*data_)[i][0] < min_val) {
      min_val = (*data_)[i][0];
    }
  }

  // setup k_means, which are initial centers for the GMM.
  //  bin points uniformly within range of first coordinate
  // TODO: allow input of these initial values, or 3D binning,
  //   or find local maxima in 3d histo, etc
  std::vector<Array1DD> k_means;
  for(unsigned int k=0;k<k_;k++) {
    k_means.push_back(Array1DD(dim_,0.));
  }
  Array1DD k_means_n = Array1DD(k_,0.);
  std::vector<int> mapping;
  IMP_LOG(VERBOSE,"min val: "<<min_val<<" max val: "<<max_val<<" denom "
          <<(max_val-min_val)/k_<<std::endl);
  for(unsigned int i=0;i<data_n_;i++) {
    int ki = (int)floor(((*data_)[i][0]-min_val)/((max_val-min_val)/k_+0.0001));
    IMP_LOG(VERBOSE,"First coord "<<(*data_)[i][0]<<" frac: "
            <<((*data_)[i][0]-min_val)/((max_val-min_val)/k_+0.0001)
            <<" ki "<<ki<<std::endl);
    k_means[ki] = algebra::internal::TNT::add(k_means[ki],(*data_)[i]);
    k_means_n[ki]++;
    mapping.push_back(ki);
  }

  std::vector<Array2DD> k_sigmas;
  for(unsigned int k=0;k<k_;k++) {
    IMP_LOG(VERBOSE,"Before normalization for k"<<k<<" = "<<k_means[k]
            <<" using factor "<<k_means_n[k]<<std::endl);
    if (k_means_n[k]>0){
      k_means[k] = algebra::internal::TNT::multiply(1./k_means_n[k],k_means[k]);
    }
    else{ //KLUGE for bad initial data
      IMP_LOG(VERBOSE,"Using bad initial positions kluge"<<std::endl);
      //k_means[k] = Array1DD(dim_,0.);
      //k_means[k][0]=1;
      k_means_n[k]++;
    }
    Array2DD sigma(dim_,dim_,0.);
    k_sigmas.push_back(sigma);
    IMP_LOG(VERBOSE,"After normalization for k"<<k
            <<" = "<<k_means[k]<<std::endl);
  }
  for(unsigned int i=0;i<data_->size();i++) {
    int ki = mapping[i];
    for(unsigned int d1=0;d1<dim_;d1++) {
      for(unsigned int d2=0;d2<dim_;d2++) {
        k_sigmas[ki][d1][d2] += ((*data_)[i][d1]-k_means[ki][d1])*
          ((*data_)[i][d2]-k_means[ki][d2]);
      }
    }
  }
  for(unsigned int k=0;k<k_;k++) {
    for(unsigned int d1=0;d1<dim_;d1++) {
      for(unsigned int d2=0;d2<dim_;d2++) {
        k_sigmas[k][d1][d2] = k_sigmas[k][d1][d2]/k_means_n[k];
      }
    }
    components_.push_back(new GaussianComponent(k_means[k],
                                                k_sigmas[k],1./k_,k));

    //    components_[k]->show();
  }
}

void GaussianMixtureModel::e_step() {
  bool valid_update=false;//check that not all responsibilities were set to 0.
  int num_valid_updates=0;
  IMP_LOG(VERBOSE,"start GMM e-step with "<<data_n_<<" data points"<<std::endl);
  double w_sum=0.;
  IMP_INTERNAL_CHECK(data_n_==data_r_.dim1(),
                     "inconsistency in GMM data members"<<std::endl);
  for(unsigned int i =0;i<data_n_;i++) {//for data points
    w_sum=0.;
    for(unsigned int j=0;j<k_;j++) {//for Gaussian components
      //data_p_[i][j] = components_[j]->pdf((*data_)[i]);
      //      std::cout<< "==data p: i:"<<i<<"j:"<<j<<" ::: "  <<
      //(*data_)[i][0] << " : " << (*data_)[i][1]
      //   << " : " <<data_p_[i][j]<<std::endl;
      //IMP::IMP_assert(not IMP::algebra::almost_equal(data_p_[i][j],0.,EPS),
      //"null probability for point " << i << " in cluster " << j <<std::endl);
      double pdf=components_[j]->pdf((*data_)[i]);
      w_sum += components_[j]->get_prior()*pdf;
      data_r_[i][j] = components_[j]->get_prior()*pdf;//data_p_[i][j];
    }
    //!!!!! TODO - decide if you should allow for 0 prob or not
    // ( needed for the hierarhcy)
//     if(IMP::algebra::almost_equal(w_sum,0.,EPS)) { //TODO - is this ok ?
//       std::cout<<"PROBLEM for data point " << i << "
//         the weighted sum of probabilities is 0 "
//             << (*data_)[i][0]<<" "<<(*data_)[i][1]
//             <<" "<<(*data_)[i][2]<<std::endl;
//       throw 1;
//       //      assert(0);
//     }
//     for(unsigned int j=0;j<k_;j++) {
//         data_r_[i][j] /= w_sum;
//      //      std::cout<<"data_r_ i:"<<i<<" j:"<< j << " : "
//              << data_r_[i][j] << std::endl;
//     }
    if(IMP::algebra::get_are_almost_equal(w_sum,0.,GMM_EPS)) {
      // IMP_LOG(VERBOSE,"PROBLEM for data point " << i <<
      //    " the weighted sum of probabilities is 0 "
      //      << (*data_)[i][0]<<" "<<(*data_)[i][1]<<" "<<
      //      (*data_)[i][2]<<std::endl);
      for(unsigned int j=0;j<k_;j++) {
        data_r_[i][j] = 0.;
      }
    }
    else {
      for(unsigned int j=0;j<k_;j++) {
        data_r_[i][j] /= w_sum;
        valid_update=true;
        ++num_valid_updates;
      }
    }
  }
  IMP_INTERNAL_CHECK(valid_update,
                     "All responsibilities were set to 0 in this e-step\n");
  IMP_LOG(VERBOSE,"Finish GMM e-step,"
          <<num_valid_updates << " out of " <<data_n_*k_<<std::endl);
}

void GaussianMixtureModel::update_log_likelihood() {
  log_lik_ = 0.;
  Array1DD log_prior(k_);
  for(unsigned int k =0;k<k_;k++) {
    log_prior[k] = std::log(components_[k]->get_prior());
  }
  for(unsigned int i =0;i<data_n_;i++) {
    double i_prob=0.;
    for(unsigned int k=0;k<k_;k++) {
      i_prob+=components_[k]->get_prior()*components_[k]->pdf((*data_)[i]);
    }
    log_lik_ += std::log(i_prob);
  }
}


/*
void GaussianMixtureModel::log_intermediates(int step_ind){
  std::stringstream ss;
  ss<<"log_inter_"<<step_ind<<".txt";
  std::vector<IMP::Particles> cluster_ps;
  for(int k=0;k<k_;k++) {
    cluster_ps.push_back(IMP::Particles());
  }
  for(unsigned int i =0;i<data_n_;i++) {
    int best_k=0;
    for(unsigned int k=1;k<k_;k++) {
      if(components_[k]->get_prior()*components_[k]->pdf((*data_)[i])>
         components_[best_k]->get_prior()*
           components_[best_k]->pdf((*data_)[i])) {
        best_k=k;
      }
    }
    IMP::Model *mdl = new IMP::Model();
    IMP::Particle *p= new IMP::Particle(mdl);
        IMP::core::XYZ::setup_particle(p,
          IMP::algebra::VectorD<3>(data_[i][0],data_[i][1],data_[i][2]));
    cluster_ps[best_k].push_back(p)
  }
  //get all of the data as particles

  for(int k=0;k<k_;k+) {
    IMP::em::SampledDensityMap* sampled_em_dens =
      IMP::em::particles2density(cluster_ps[k],
                            resolution,apix,3,
                                 IMP::core::XYZR::get_default_radius_key(),
                                 IMP::em::Voxel::get_density_key());
    IMP::em::MRCReaderWriter mrw;
    IMP::em::write_map(sampled_em_dens,ss.c_str(),mrw);
  }
}

*/
void GaussianMixtureModel::m_step() {
  update_log_likelihood();
  for(unsigned int j=0;j<k_;j++) {
    components_[j]->update(*data_,data_r_);
  }
}

void GaussianMixtureModel::change_data(statistics::internal::DataPoints *data) {
    full_data_ = data;
    data_=full_data_->get_data();
    IMP_LOG(VERBOSE,"Changing data for GMM with dim: " << dim_
            << " the new dim is : " <<  (*data_)[0].dim1() <<std::endl);
    IMP_INTERNAL_CHECK(dim_ == (*data_)[0].dim1(),
                       "The data has a different dimension than the current one"
                       <<std::endl);
    //dim_ = (*data_)[0].dim1();
    data_n_ = data_->size();
    data_r_ = Array2DD(data_n_,k_,1./k_);
    data_p_ = Array2DD(data_n_,k_);
}


//! Run Expectation-Maximization
void GaussianMixtureModel::run() {
  unsigned int step_i = 0;
  //e_step(); //TODO - how to avoid this extra e_step ?
  double old_log_lik = INT_MAX;//log_lik_;
  //EM loop
  bool has_converged = false;
  boost::progress_display show_progress(num_steps_);//MAX_ITER
  do {
    ++show_progress;
    IMP_LOG(VERBOSE,"GaussianMixtureModel::run iteration step : "
            << step_i << std::endl);
    step_i++;
    if (step_i>num_steps_){//TODO - use MAXITER){
      IMP_LOG(VERBOSE,
              "EM procedure ended. Max number of iterations has been reached."
              << std::endl);
      has_converged = true;
    }
    e_step();
    m_step();
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE,"GMM-EM iteration: "
              << step_i << "log_lik: " << log_lik_ << std::endl);
      IMP_LOG_WRITE(VERBOSE,show());
    }
    // the algorithm has converged
    if(IMP::algebra::get_are_almost_equal(log_lik_,old_log_lik,GMM_EPS)) {
      //TODO - return
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE,"GMM-EM coverged at: "
                <<log_lik_ << " after " << step_i << " steps " <<std::endl);
        IMP_LOG_WRITE(VERBOSE,show());
      }
      has_converged=true;
    }
    old_log_lik = log_lik_;
    IMP_LOG(TERSE,"After step:"<<step_i
            <<" LOG_LIKELIHOOD " << log_lik_ <<std::endl);
    //write segments as mrc files
  } while(not has_converged);
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"GMM-EM coverged "<<std::endl);
    IMP_LOG_WRITE(VERBOSE,show());
  }
  //update the progress bar incase not all iteration were performed
  while (step_i<num_steps_){//TODO - use MAXITER){
    step_i++;
    ++show_progress;
  }
}
void GaussianMixtureModel::show(std::ostream &out) const {
  for (unsigned int i=0;i<components_.size();i++) {
    components_[i]->show_single_line(out);
  }
}

Array2DD GaussianMixtureModel::get_data_sigma() const {
  Array2DD sigma(dim_,dim_,0.);
  Array1DD data_mean = Array1DD(dim_,0.);
  for(unsigned int i=0;i<data_->size();i++) {
    data_mean = algebra::internal::TNT::add(data_mean ,(*data_)[i]);
  }
  data_mean = algebra::internal::TNT::multiply(1./data_->size(),data_mean);
  for(unsigned int i=0;i<data_->size();i++) {
    for(unsigned int d1=0;d1<dim_;d1++) {
      for(unsigned int d2=0;d2<dim_;d2++) {
        sigma[d1][d2] +=
          ((*data_)[i][d1]-data_mean[d1])*((*data_)[i][d2]-data_mean[d2]);
      }
    }
  }
  for(unsigned int d1=0;d1<dim_;d1++) {
    for(unsigned int d2=0;d2<dim_;d2++) {
      sigma[d1][d2] = sigma[d1][d2]/data_->size();
    }
    sigma[d1][d1] += 1e-5f;
  }
  return sigma;
}
IMPSTATISTICS_END_NAMESPACE

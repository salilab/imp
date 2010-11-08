/**
 *  \file VQClustering.cpp
 *  \brief Vector quantization clustering. Based on Wriggers et at, JMB 1998
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/VQClustering.h>
#include <IMP/multifit/DataPoints.h>
#include <IMP/em/converters.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/tnt_array1d_utils.h>
#include <IMP/algebra/vector_search.h>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
IMPMULTIFIT_BEGIN_NAMESPACE
using namespace algebra::internal;

//! help function for sorting centers by their distance to a data point
class CenterSorter {
public:
  CenterSorter(){}
  CenterSorter(Array1DD *p,Array1DD_VEC *cs):p_(p),cs_(cs){}
  void set_point(Array1DD *p) {p_=p;}
  void set_centers(Array1DD_VEC *cs) {cs_=cs;}
  bool operator() (int c_i,int c_j) {
    double d_i=TNT::dot_product(*p_-(*cs_)[c_i],*p_-(*cs_)[c_i]);
    double d_j=TNT::dot_product(*p_-(*cs_)[c_j],*p_-(*cs_)[c_j]);
    if (d_i<d_j) return true;
    else return false;
  }
protected:
  Array1DD *p_;
  Array1DD_VEC *cs_;
};

VQClustering::VQClustering() {
  is_set_ = false;
}
VQClustering::VQClustering(DataPoints *data, int k)
  : k_(k) {
  //store relevate particle data in a more efficeint data structure
  full_data_ = data;
  data_ = full_data_->get_data();
  IMP_INTERNAL_CHECK(data_->size()>0,"no data points to cluster");
  dim_ = (*data_)[0].dim1();
  number_of_runs_ =15;
  number_of_steps_ = 100000;
  ei_ = 0.1;
  ef_ = 0.001;
  li_ = 0.2 * k_;
  lf_ = 0.02;
  is_set_ = false;
}

void VQClustering::sample_data_point(Array1DD &p) {
  p = full_data_->sample();
}

void VQClustering::center_sampling(Array1DD_VEC *centers_sample) {
  for(int i=0;i<k_;i++){
    Array1DD p_new;
    sample_data_point(p_new);
    centers_sample->push_back(p_new);
  }
}


void VQClustering::sampling(Array1DD_VEC *tracking) {
  double epsilon;//neuron plasticity
  double lambda;//approximated width
  //order_track and order_centers_indexes are data structures used to sort
  //the centers
  //according to their distance from a randomly selected data point
  std::vector<int> order_track;
  std::vector<int> order_centers_indexes;
  order_track.insert(order_track.end(),k_,0);
  order_centers_indexes.insert(order_centers_indexes.end(),k_,0);
  CenterSorter sorter;
  boost::progress_display show_progress(number_of_runs_);
  Array1DD_VEC centers_sample;
  for (int run_ind=0; run_ind<number_of_runs_; ++run_ind) {
    ++show_progress;
    IMP_LOG(IMP::VERBOSE,"TRN clustering run number : " << run_ind <<std::endl);
    //randomly sample centers from the data points
    //data_->sample_centers(centers_sample,k_,random_offset_,false);
    center_sampling(&centers_sample);

    //update the centers according to the distance from set of number_of_steps
    //randomly selected data points
    for (int step_ind=0; step_ind<number_of_steps_; step_ind++) {
      Array1DD rand_data_p;
      sample_data_point(rand_data_p);
      //sort all of the centers from the closest to the farthest
      //to the randomly selected data point
      sorter.set_point(&rand_data_p);sorter.set_centers(&centers_sample);
      for (int i=0; i<k_; ++i) order_track[i]=i;
      std::sort(order_track.begin(),order_track.end(),sorter);
      //order_centers_indexes holds the center indexes according to their
      //distance from the random point ( low to high)
      for (int i=0; i<k_; ++i) {
        order_centers_indexes[i]=order_track[i];
      }
      epsilon = ei_ * exp((1.*step_ind/number_of_steps_)*log(ef_/ei_));
      lambda =  li_ * exp((1.*step_ind/number_of_steps_)*log(lf_/li_));

      //update the centers
      for (int i=0; i<k_; ++i) {
        Array1DD *cen = &(centers_sample[order_centers_indexes[i]]);
        double t = epsilon * exp (-i/lambda);
        for (int j=0; j<dim_; ++j) {//all attributes
          (*cen)[j] += t * (rand_data_p[j] - (*cen)[j]);
        }
      }

//todo - add which particles belong to which center
    } //end steps
    //keep the centers of the current run in the tracking vector
    for (int i=0; i<k_; ++i) {
      tracking->push_back(centers_sample[i]);
    }
  }
}

void VQClustering::clustering(Array1DD_VEC *tracking, Array1DD_VEC *centers) {
  std::vector<std::vector<int> > closest_center;
  double min_dist, curr_dist;
  int closest_center_ind=0;
  Array1DD att_sum(dim_,0.);
  double wdiff = INT_MAX;
  Array1DD_VEC last_centers;
  for(int i=0;i<k_;i++) {
    last_centers.push_back(Array1DD(dim_));
  }
  // loop until clustering convergance
  do {
    closest_center.clear();
    for (int i=0; i<k_; ++i) {
      closest_center.push_back(std::vector<int>());
    }
    // for each sampling center in tracking find the closest center
    // in current centers. This information is stored in closest_center vector
    for (unsigned int j=0;j<tracking->size();j++) {
      min_dist = 1e20;
      for (int cen_ind=0;cen_ind<k_;cen_ind++){
        curr_dist = TNT::dot_product(
            (*centers)[cen_ind]-(*tracking)[j],
            (*centers)[cen_ind]-(*tracking)[j]);
        if (curr_dist < min_dist) {
          min_dist = curr_dist;
          closest_center_ind = cen_ind;
        }}
      closest_center[closest_center_ind].push_back((int)j);
    }
    // According to the closest_center logging we compute
    // the refined centers of the k clusters
    for (int cen_ind=0;cen_ind<k_;cen_ind++){
      Array1DD *cen   = &((*centers)[cen_ind]);
      last_centers[cen_ind]=(*centers)[cen_ind];
      //update att_sum data
      for(int i=0;i<dim_;i++) {att_sum[i]=0.;}
      for (unsigned int j=0; j<closest_center[cen_ind].size(); j++) {
        Array1DD p= (*tracking)[closest_center[cen_ind][j]];
        for(int d=0;d<dim_;d++){
          att_sum[d] += p[d];
        }}
      if (closest_center[cen_ind].size()>0) {
        //check if center with cen_ind is going to be update
        for(int d=0;d<dim_;d++){
          (*cen)[d] = att_sum[d] / closest_center[cen_ind].size();
        }}}
    // compute stopping criterion
    wdiff = 0.;
    for (int i=0; i<k_; ++i)  {
      wdiff += TNT::dot_product((*centers)[i]-last_centers[i],
                                (*centers)[i]-last_centers[i]);
    }
    wdiff = sqrt(wdiff/(double)k_);
  } while (wdiff > 1e-5);
}

void VQClustering::set_assignments(){
  //index the centers
  algebra::Vector3Ds all_cen;
  for(int i=0;i<k_;i++) {
    all_cen.push_back(algebra::Vector3D(centers_[i][0],
                                        centers_[i][1],
                                        centers_[i][2]));
  }
  algebra::NearestNeighborD<3> nn(all_cen);
  double min_dist,curr_dist;
  assignment_.clear();
  for(unsigned int j=0;j<data_->size();j++) {
    algebra::Vector3D point((*data_)[j][0],(*data_)[j][1],(*data_)[j][2]);
    //    int closest_cen = nn.get_nearest_neighbor(point);
    //TODO- does not work for now, replace once corrected
    int closest_cen;int min_dist=INT_MAX;
    for(int l=0;l<all_cen.size();l++){
      if (algebra::get_squared_distance(all_cen[l],point)<min_dist) {
        min_dist=algebra::get_squared_distance(all_cen[l],point);
        closest_cen=l;
      }
    }
    assignment_.push_back(closest_cen);
  }
}

void VQClustering::run(DataPoints *starting_centers){
  //tracking keeps information of all suggested centers throught the algorithm
  Array1DD_VEC tracking;
  IMP_LOG(IMP::VERBOSE,"VQClustering::run before sampling"<<std::endl);
  sampling(&tracking);
  IMP_LOG(IMP::VERBOSE,"VQClustering::run after sampling"<<std::endl);
  centers_.clear();
  //the initial centers to the clustering are the results of the first run
  //TODO - maybe we can improve that ?
  if (starting_centers == NULL) {
  for(int i=0;i<k_;i++) {
    centers_.push_back(tracking[i]);
  }
  }
  else {
    IMP_INTERNAL_CHECK(k_==starting_centers->get_number_of_data_points(),
                       "The starting centers are not of size K"<<std::endl);
    for(int i=0;i<k_;i++) {
      centers_.push_back((*(starting_centers->get_data()))[i]);
    }
  }
  IMP_LOG(IMP::VERBOSE,"VQClustering::run before clustering"<<std::endl);
  clustering(&tracking,&centers_);
  IMP_LOG(IMP::VERBOSE,"VQClustering::run after clustering"<<std::endl);
  set_assignments();
  //  set_centers_as_particles();
  is_set_ = true;
}
void VQClustering::set_fast_clustering() {
  number_of_runs_ = 10;
  number_of_steps_ = 1000;
}
IMPMULTIFIT_END_NAMESPACE

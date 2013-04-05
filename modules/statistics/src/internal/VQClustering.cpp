/**
 *  \file VQClustering.cpp
 *  \brief Vector quantization clustering. Based on Wriggers et at, JMB 1998
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/statistics/internal/VQClustering.h>
#include <IMP/statistics/internal/DataPoints.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/tnt_array1d_utils.h>
#include <IMP/algebra/vector_search.h>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
using namespace algebra::internal;


typedef std::pair<double,unsigned int> ValInd;
typedef std::vector<ValInd> ValInds;

namespace {
  bool comp_first_smaller_than_second(const ValInd& a,
                                      const ValInd& b) {
    return a.first < b.first;
  }
}
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

VQClustering::VQClustering(){
  is_set_ = false;
  show_status_bar_=true;
}
VQClustering::VQClustering(DataPoints *data, int k)
  : k_(k) {
  //store relevate particle data in a more efficeint data structure
  show_status_bar_=true;
  full_data_ = data;
  data_ = full_data_->get_data();
  IMP_INTERNAL_CHECK(data_->size()>0,"no data points to cluster");
  dim_ = (*data_)[0].dim1();
  par_=VQClusteringParameters(dim_,k_);
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
  double log_ef_ei=log(par_.ef_/par_.ei_);
  double log_lf_li=log(par_.lf_/par_.li_);

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
  int show_number_of_runs=0;
  if (show_status_bar_) {
    show_number_of_runs=par_.number_of_runs_;
  }
  boost::progress_display show_progress(show_number_of_runs);
  Array1DD_VEC centers_sample;
  for (int run_ind=0; run_ind<par_.number_of_runs_; ++run_ind) {
    if (show_status_bar_) {
      ++show_progress;
    }
    IMP_LOG_VERBOSE("TRN clustering run number : " << run_ind <<std::endl);
    //randomly sample centers from the data points
    center_sampling(&centers_sample);

    //update the centers according to the distance from set of number_of_steps
    //randomly selected data points
    for (int step_ind=0; step_ind<par_.number_of_steps_; step_ind++) {
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
      epsilon = par_.ei_ * exp((1.*step_ind/par_.number_of_steps_)*log_ef_ei);
      lambda =  par_.li_ * exp((1.*step_ind/par_.number_of_steps_)*log_lf_li);

      //update the centers
      for (int i=0; i<k_; ++i) {
        Array1DD *cen = &(centers_sample[order_centers_indexes[i]]);
        double t = epsilon * exp (-i/lambda);
        for (int j=0; j<dim_; ++j) {//all attributes
          (*cen)[j] += t * (rand_data_p[j] - (*cen)[j]);
        }
      }
      //todo - add which particles belong to which center
      //update the centers to be eq space -- TODO --
    } //end steps
    //keep the centers of the current run in the tracking vector
    for (int i=0; i<k_; ++i) {
      tracking->push_back(centers_sample[i]);
    }
  }//end run iterations
if (par_.eq_clusters_) {
  Array1DD_VEC centers_sample_eq;
  get_eq_centers(&centers_sample,&centers_sample_eq);
  std::cout<<"Updates centers:"<<std::endl;
  for(int ll=0;ll<k_;ll++) {
    std::cout<< centers_sample_eq[ll][0]
             <<" "<< centers_sample_eq[ll][1]
             <<" "<< centers_sample_eq[ll][2]<<" "<< std::endl;
  }
  std::cout<<"Updates centers:"<<std::endl;
  for(int ll=0;ll<k_;ll++) {
    std::cout<<"From "<<centers_sample[ll][0]
             <<" "<<centers_sample[ll][1]
             <<" "<<centers_sample[ll][2]
             <<" To "<< centers_sample_eq[ll][0]
             <<" "<< centers_sample_eq[ll][1]
             <<" "<< centers_sample_eq[ll][2]<<" "<< std::endl;
  }
  centers_sample_eq=centers_sample;
 }
}//end sampling

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

//Return eq size version of the current centers
void VQClustering::get_eq_centers(Array1DD_VEC *centers,
                                  Array1DD_VEC *eq_centers) {
  const Array1DD_VEC * data = full_data_->get_data();
  int num_points = full_data_->get_number_of_data_points();
  unsigned int eq_size = num_points/k_;
  std::cout<<"Number of points:"<<num_points<<" eq_size:"<<eq_size<<std::endl;
  //TODO - think of eq mass option
  //calculate all pairwise distances
  ValInds distances(k_*num_points);
  //TODO - only calculate distances for close centers, think of using knn
  //TODO - print all of the unassigned anchors
  //TODO - use the GMM
  //TODO - add the edges
  for(int i=0;i<k_;i++) {
    for(int j=0;j<num_points;j++) {
      Array1DD a1=(*centers)[i];
      Array1DD a2=(*data)[j];
      distances[i*num_points+j]=
        std::make_pair(
        TNT::dot_product(
                         (*centers)[i]-(*data)[j],
                         (*centers)[i]-(*data)[j]),
        i*num_points+j);
    }
  }
  //now create the assignment
  std::vector<Ints> assignments(k_);
  int centers_full=0;
  int points_visited=0;
  bool not_done=true;
  //make a heap of the array
  std::make_heap(distances.begin(),distances.end(),
                 comp_first_smaller_than_second);
  std::sort_heap(distances.begin(),distances.end(),
                 comp_first_smaller_than_second);
  unsigned int heap_size=distances.size();
  while (not_done) {
    int min_ind=distances[0].second;
    int center_ind=min_ind/num_points;
    int point_ind=min_ind-center_ind*num_points;
    assignments[center_ind].push_back(point_ind);
    std::pop_heap(distances.begin(),distances.begin()+heap_size,
                  comp_first_smaller_than_second);
    --heap_size;
    std::sort_heap(distances.begin(),distances.begin()+heap_size,
                   comp_first_smaller_than_second);
    if (assignments[center_ind].size()==eq_size) {
      std::cout<<"remove points for center:"<<center_ind<<std::endl;
      for(unsigned int i=0;i<heap_size;i++) {
        int counter=0;
        if ((((int)distances[i].second)>=center_ind*num_points) &&
            (((int)distances[i].second)<(center_ind+1)*num_points)) {
          distances[i].first=INT_MAX;
          counter++;
        }
        heap_size-=counter;
      }
      //order is now wrong
      // std::make_heap(distances.begin(),distances.end(),
      //                comp_first_smaller_than_second);
      std::sort_heap(distances.begin(),distances.begin()+heap_size,
                     comp_first_smaller_than_second);
      ++centers_full;
    }
    ++points_visited;
    if (centers_full==k_) {
      std::cout<<"END eq loop: centers assignments are full"<<std::endl;
      not_done=false;}
    if (points_visited==num_points) {
      std::cout<<"END eq loop: visited all points"<<std::endl;
      not_done=false;}
    if (heap_size==0) {
      std::cout<<"END eq loop: heap is empty"<<std::endl;
      not_done=false;}
  }
  //recalculate centers
  for(int i=0;i<k_;i++) {
    algebra::Vector3D cen(0.,0.,0.);
    for (unsigned int j=0;j<assignments[i].size();j++) {
      cen+=algebra::Vector3D((*data)[assignments[i][j]][0],
                             (*data)[assignments[i][j]][1],
                             (*data)[assignments[i][j]][2]);
    }
    cen/=assignments[i].size();
    std::cout<<"new cen:"<<cen<<std::endl;
    Array1DD v(dim_);
    for(int j=0;j<dim_;j++){
      v[j]=cen[j];
    }
    eq_centers->push_back(v);
  }
  std::cout<<"before returning"<<std::endl;
  for(int i=0;i<k_;i++) {
    std::cout<<"cen:"<<i<<" : " << std::endl;
    for(int j=0;j<dim_;j++){
      std::cout<<(*eq_centers)[i][j]<<" ";
    }
    std::cout<<std::endl;
  }
}


void VQClustering::set_assignments(){
  //index the centers
  algebra::Vector3Ds all_cen;
  for(int i=0;i<k_;i++) {
    all_cen.push_back(algebra::Vector3D(centers_[i][0],
                                        centers_[i][1],
                                        centers_[i][2]));
  }
  IMP_NEW(algebra::NearestNeighborD<3>, nn,(all_cen));
  assignment_.clear();
  assignment_.insert(assignment_.end(),data_->size(),0);
  for(unsigned int j=0;j<data_->size();j++) {
    algebra::Vector3D point((*data_)[j][0],(*data_)[j][1],(*data_)[j][2]);
    int closest_cen = nn->get_nearest_neighbor(point);
    //-----
    //debug
    int min_ind=0;
    double min_dist=999999;
    for(unsigned int kk=0;kk<all_cen.size();kk++) {
      if (min_dist>algebra::get_distance(point,all_cen[kk])) {
        min_dist=algebra::get_distance(point,all_cen[kk]);
        min_ind=kk;
      }
    }
    if (min_ind!=closest_cen) {
      std::cerr<<"Center for "<<j<<" does not match"<<std::endl;
    }
    if (min_dist>100) {
      std::cerr<<"Outlier for cneter "<<closest_cen<<std::endl;
    }
    //-----
    assignment_[j]=closest_cen;
  }
}

void VQClustering::run(DataPoints *starting_centers){
  //tracking keeps information of all suggested centers throught the algorithm
  Array1DD_VEC tracking;
  IMP_LOG_VERBOSE("VQClustering::run before sampling"<<std::endl);
  sampling(&tracking);
  IMP_LOG_VERBOSE("VQClustering::run after sampling"<<std::endl);
  centers_.clear();
  //the initial centers to the clustering are the results of the first run
  //TODO - maybe we can improve that ?
  if (starting_centers == nullptr) {
    for(int i=0;i<k_;i++) {
      centers_.push_back(tracking[i]);
    }
  }
  else {
    //add all precalculated centers
    for(int i=0;i<starting_centers->get_number_of_data_points();i++) {
      centers_.push_back((*(starting_centers->get_data()))[i]);
    }
    //rest are sampled
    for(int i=starting_centers->get_number_of_data_points();i<k_;i++) {
      centers_.push_back(tracking[i]);
    }
  }
  IMP_LOG_VERBOSE("VQClustering::run before clustering"<<std::endl);
  clustering(&tracking,&centers_);
  IMP_LOG_VERBOSE("VQClustering::run after clustering"<<std::endl);
  set_assignments();
  //  set_centers_as_particles();
  is_set_ = true;
}
void VQClustering::set_fast_clustering() {
  par_.number_of_runs_ = 10;
  par_.number_of_steps_ = 1000;
}
IMPSTATISTICS_END_INTERNAL_NAMESPACE

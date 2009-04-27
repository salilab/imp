/**
 * \file KMFilterCenters.cpp
 * \brief Provides efficient algorithm for compuing distortions
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/domino/KMFilterCenters.h>
#include <IMP/domino/random_generator.h>
IMPDOMINO_BEGIN_NAMESPACE
KMFilterCenters::KMFilterCenters(int k, KMData* data,
                                 KMPointArray *ini_cen_arr,double df)
  : KMCenters(k,data) {
  damp_factor_ = df;
  ini_cen_arr_ = ini_cen_arr;
  sums_ = allocate_points(k,data_points_->get_dim());
  sum_sqs_.insert(sum_sqs_.end(),k,0.);
  weights_.insert(weights_.end(),k,0);
  dists_.insert(dists_.end(),k,0);
  curr_dist_ = INT_MAX;
  tree_ = new KMCentersTree(data_points_,this);
  invalidate();// distortions are initially invalid
}
// assignment operator
KMFilterCenters& KMFilterCenters::operator=(const KMFilterCenters &other) {
  if (this != &other) {
    KMCenters::operator=(other);
    //copy sums
    sums_ = new KMPointArray();
    copy_points(other.sums_,sums_);
    //copy sum_sqs
    copy_point(&other.sum_sqs_,&sum_sqs_);
    //copy weights
    for(unsigned int i=0;i<other.weights_.size();i++) {
      weights_.push_back(other.weights_[i]);
    }
    //copy ini_cen_arr
    if (other.ini_cen_arr_ == NULL) {
      ini_cen_arr_ = NULL;
    }
    else{
      ini_cen_arr_ = new KMPointArray();
      copy_points(other.ini_cen_arr_,ini_cen_arr_);
    }
    //copy distortions
    copy_point(&other.dists_,&dists_);
    //copy current total distortion
    curr_dist_ = other.curr_dist_;
    //copy valid flag
    valid_ = other.valid_;
    //copy dampening factor
    damp_factor_ = other.damp_factor_;
    tree_ = other.tree_;
  }
  return *this;
}
// copy constructor
KMFilterCenters::KMFilterCenters(const KMFilterCenters &other)
  : KMCenters(other){
  //copy sums
  sums_ = new KMPointArray();
  copy_points(other.sums_,sums_);
  //copy sum_sqs
  copy_point(&other.sum_sqs_,&sum_sqs_);
  //copy weights
  for(unsigned int i=0;i<other.weights_.size();i++) {
    weights_.push_back(other.weights_[i]);
  }
  //copy ini_cen_arr
  ini_cen_arr_ = new KMPointArray();
  copy_points(other.ini_cen_arr_,ini_cen_arr_);
  //copy distortions
  copy_point(&other.dists_,&dists_);
  //copy current total distortion
  curr_dist_ = other.curr_dist_;
  //copy valid flag
  valid_ = other.valid_;
  //copy dampening factor
  damp_factor_ = other.damp_factor_;
  tree_ = other.tree_;
}

KMFilterCenters::~KMFilterCenters() {
  deallocate_points(sums_);
  deallocate_points(ini_cen_arr_);
  delete tree_;
}

void KMFilterCenters::compute_distortion()
{
  IMP_assert(tree_ != NULL,"The tree should be initialized");
  IMP_assert(sums_!=NULL,"sums_ were not allocated\n");
  tree_->get_neighbors(sums_,&sum_sqs_,&weights_);
  curr_dist_=0.;
  for (int j = 0; j < get_number_of_centers(); j++) {
    double c_dot_c = 0.;
    double c_dot_s = 0.;
    // compute dot products
    for (int d = 0; d < data_points_->get_dim(); d++) {
      double c_val = (*((*centers_)[j]))[d];
      c_dot_c +=  c_val*c_val;
      c_dot_s += c_val * (*((*sums_)[j]))[d];
    }
    // final distortion
    dists_[j] = sum_sqs_[j] - 2*c_dot_s + weights_[j]*c_dot_c;
    curr_dist_ += dists_[j];
  }
  validate();
}

void KMFilterCenters::get_assignments(std::vector<int> &close_center)
{
  IMP_assert(tree_ != NULL,"The tree is NULL");
  tree_->get_assignments(close_center);
}

void KMFilterCenters::move_to_centroid()
{
    if (!valid_) compute_distortion();
    for (int j = 0; j < get_number_of_centers(); j++) {
      KMPoint *p = (*centers_) [j];
      int wgt = weights_[j];
      if (wgt > 0) {
        for (int d = 0; d < get_dim(); d++) {
          (*p)[d] = (1 - damp_factor_) * (*p)[d] +
          damp_factor_ * (*((*sums_)[j]))[d]/wgt;
        }
      }
    }
    invalidate();
}
void KMFilterCenters::show(std::ostream& out) const{
  for (int j = 0; j < get_number_of_centers(); j++) {
    out << "    " << std::setw(4) << j << "\t";
    print_point(*((*centers_)[j]), out);
    out << " dist = " << std::setw(8) << dists_[j] << std::endl;
  }
}
IMPDOMINO_END_NAMESPACE

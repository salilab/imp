/**
 * \file KMFilterCenters.cpp
 * \brief Provides efficient algorithm for compuing distortions
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/statistics/KMFilterCenters.h>
#include <IMP/statistics/random_generator.h>
IMPSTATISTICS_BEGIN_NAMESPACE
KMFilterCenters::KMFilterCenters(){}
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
KMFilterCenters::~KMFilterCenters() {
  deallocate_points(sums_);
  deallocate_points(ini_cen_arr_);
  if (tree_ != NULL)
    delete tree_;
}

void KMFilterCenters::invalidate() {
  IMP_LOG_WRITE(VERBOSE,show());
  clear_data();
  valid_ = false;
}

void KMFilterCenters::generate_random_centers(int k) {
  if (ini_cen_arr_ != NULL) {
    IMP_LOG(VERBOSE,"KMFilterCenters::generate_random_centers"
    <<" with initial points"<<std::endl);
    for (int i=0;i<k;i++) {
      for(int j=0;j<data_points_->get_dim();j++) {
        KMPoint *p= (*ini_cen_arr_)[i];
        (*(*centers_)[i])[j]=
        random_uniform((*(p))[j]-20.,(*(p))[j]+20.);
        //kmCopyPt(pts->getDim(),(*ini_cen_arr_)[i],ctrs[i]);
      }//for j
    }//for i
  }
  else {
    IMP_LOG(VERBOSE,"KMFilterCenters::generate_random_centers"
    <<" without initial points"<<std::endl);
    data_points_->sample_centers(centers_,k,false);
  }
  invalidate();
}
void KMFilterCenters::clear_data() {
  if (sums_ != NULL) {
    for(unsigned int i=0;i<sums_->size();i++) {
      KMPoint *p = (*sums_)[i];
      if (p != NULL) {
        for(unsigned int j=0;j<p->size();j++) {
          (*p)[j]=0.;
        }
      }
    }
  }
  for(unsigned int i=0;i<sum_sqs_.size();i++) {
    sum_sqs_[i]=0.;
  }
  for(unsigned int i=0;i<weights_.size();i++) {
    weights_[i]=0;
  }
}
void KMFilterCenters::compute_distortion()
{
  IMP_assert(tree_ != NULL,"The tree should be initialized");
  IMP_assert(sums_!=NULL,"sums_ were not allocated\n");
  clear_data();
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
void KMFilterCenters::show(std::ostream& out) {
  if (!valid_) compute_distortion();
  for (int j = 0; j < get_number_of_centers(); j++) {
    out << "    " << std::setw(4) << j << "\t";
    print_point(*((*centers_)[j]), out);
    out << " dist = " << std::setw(8) << dists_[j] <<
           " weight = " << std::setw(8) << weights_[j] <<
           std::endl;
  }
  tree_->show(out);
}
IMPSTATISTICS_END_NAMESPACE

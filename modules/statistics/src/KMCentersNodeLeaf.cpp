/**
 * \file KMCenetersNode.cpp
 * \brief leaf node of the kc-tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/statistics/KMCentersNodeLeaf.h>
#include <IMP/statistics/internal/random_generator.h>
#include <limits.h>
IMPSTATISTICS_BEGIN_NAMESPACE
void KMCentersNodeLeaf::compute_sums() {
 sum_sq_ = 0.;
 KMData *data = centers_->get_data();
 for (unsigned int i = 0; i < data_ps_.size(); i++) {
   KMPoint *p = (*data)[data_ps_[i]];
   for (int d = 0; d < centers_->get_dim(); d++) {
     double coord = (*p)[d];
     sum_[d] += coord;
     sum_sq_ += coord * coord;
   }
 }
}

KMPoint KMCentersNodeLeaf::sample_center() {
  int ri = internal::random_int(n_data_);
  return *((*(centers_->get_data()))[data_ps_[ri]]);
}

void KMCentersNodeLeaf::show(std::ostream& out) const{
  out << "    ";
  for (int i = 0; i < level_; i++)
    out << ".";
  out << "Leaf n=" << n_data_ << " <";
  for (int j = 0; j < n_data_; j++) {
    out << data_ps_[j] << ", ";
  }
  out << ">" << " sum=";  print_point(sum_, out);
  out << " ss=" << sum_sq_ << std::endl;
}
void KMCentersNodeLeaf::get_neighbors(const std::vector<int> &cands,
    KMPointArray *sums, KMPoint *sum_sqs,std::vector<int> *weights)
{
  IMP_LOG(VERBOSE,
  "KMCentersNodeLeaf::get_neighbors for " << cands.size() << " candidates\n");
  //if only one candidate left, post points as neighbors
  if (cands.size() == 1) {
    IMP_LOG(VERBOSE,
    "KMCentersNodeLeaf::get_neighbors the particles are associated"
    <<" with center : " << cands[0] << "\n");
    post_neighbor(sums, sum_sqs, weights, cands[0]);
    return;
  }
  //find the closest centers for each point in the bucket
  IMP_assert((unsigned int)n_data_ == data_ps_.size(),
             "KMCentersNodeLeaf::get_neighbors inconsistency in sizes \n");
  for (int i=0;i<n_data_;i++) {
    KMPoint *data_p = (*(centers_->get_data()))[data_ps_[i]];
    int min_k= 0;
    double min_dist = km_distance2(*((*centers_)[cands[0]]), *data_p);
    for (unsigned int j = 1; j < cands.size(); j++) {
      double dist = km_distance2(*((*centers_)[cands[j]]), *data_p);
      if (dist < min_dist) {
        min_dist = dist;
        min_k = j;
      }
    }
    IMP_LOG(VERBOSE,
    "KMCentersNodeLeaf::get_neighbors data point "<< data_ps_[i] <<
    " is associated" <<" with center : " << cands[min_k] << "\n");
    post_one_neighbor(sums, sum_sqs, weights, cands[min_k],*data_p);
  }
}
void KMCentersNodeLeaf::get_assignments(const std::vector<int> &cands,
  std::vector<int> &close_center){
  KMData *data = centers_->get_data();
  for (int i = 0; i < n_data_; i++) {
    //find the closest center to each data point associated to the node
    double min_dist = INT_MAX;
    int closest_ind;
    KMPoint *p = (*data)[data_ps_[i]];
    for (unsigned int j = 0; j < cands.size(); j++) {
      double dist = km_distance2(*((*centers_)[cands[j]]),*p);
      if (dist < min_dist) {
        min_dist = dist;
        closest_ind = j;
      }
    }
    close_center[data_ps_[i]] = cands[closest_ind];
  }
}



IMPSTATISTICS_END_NAMESPACE

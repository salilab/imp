/**
 * \file KMCenetersNode.cpp
 * \brief leaf node of the kc-tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/domino/KMCentersNodeLeaf.h>
#include <IMP/domino/random_generator.h>
#include <limits.h>
IMPDOMINO_BEGIN_NAMESPACE
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
  int ri = random_int(n_data_);
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
  //if only one candidate left, post points as neighbors
  if (cands.size() == 1) {
    post_neighbor(sums, sum_sqs, weights, cands[0]);
    return;
  }
  IMP_assert(cands.size() == 1,"not sure how to handle that !!");
//   //find the closest centers for each point in the bucket
//   KMData *data = centers_->get_data();
//   for (int i = 0; i < n_data_; i++) {
//     double min_dist = MAX_INT;
//     int closest_ind;
//     KMPoint *p = (*data)[data_ps_[i]];
//     for (int j = 0; j < cands.size(); j++) {
//       double dist = km_distance2((*centers_)[cands[j]],p);
//       if (dist < min_dist) {
//       min_dist = dist;
//       closest_ind = j;
//       }
//     }
//     post_neighbor(sums, sum_sqs, weights, cands[closest_ind]);
//   }
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



IMPDOMINO_END_NAMESPACE

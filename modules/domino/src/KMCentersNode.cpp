/**
 * \file KMCenetersNode.cpp
 * \brief generic node of the kc-tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/domino/KMCentersNode.h>
IMPDOMINO_BEGIN_NAMESPACE

KMCentersNode::KMCentersNode(const KMRectangle &bb,
                             KMCenters *centers,int level)
  : bnd_box_(bb),centers_(centers),level_(level){
  int dim = bnd_box_.get_dim();
  sum_.insert(sum_.end(),dim,0);
  sum_sq_ = 0;
  n_data_=0;
}

KMPoint KMCentersNode::get_mid_point() const {
  KMPoint p;
  const KMPoint *l,*h;
  l = bnd_box_.get_point(0);
  h = bnd_box_.get_point(1);
  for (unsigned int d = 0; d < l->size(); d++) {
    p[d] = ((*l)[d] + (*h)[d])/2;
  }
  return p;
}

KMCentersNode::~KMCentersNode()
{}

void KMCentersNode::compute_close_centers(
   const std::vector<int> &candidate_centers_inds,
   std::vector<int> *close_centers_inds) {
  //first we calculate the center that is closest to the middle of the
  //bounding box
  int mid_center_ind = mid_center(candidate_centers_inds);
  KMPoint *mid_cen = (*centers_)[mid_center_ind];
  double box_dot = 0.; // holds (p-c').(c-c')
  double cc_dot = 0.;  // holds (c-c').(c-c')
  for(std::vector<int>::const_iterator it = candidate_centers_inds.begin();
      it != candidate_centers_inds.end(); it++) {
    if (*it == mid_center_ind) {
      close_centers_inds->push_back(*it);
    }
    else {
      KMPoint *cand_cen = (*centers_)[*it];
      KMPoint closest_vertex = bnd_box_.find_closest_vertex(*cand_cen);
      box_dot = 0.;
      cc_dot = 0.;
      for (int d = 0; d < bnd_box_.get_dim(); d++) {
        double cc_comp = (*cand_cen)[d] - (*mid_cen)[d];
        cc_dot += cc_comp * cc_comp; // increment dot product
        box_dot=cc_comp*(closest_vertex[d]-(*mid_cen)[d]);
      }
      if (cc_dot<2*box_dot) {
        close_centers_inds->push_back(*it);
      }
    }
  }
}
//! Assign neighbors to center
/**
/param[in] p the node posting
/param[in] sum the sum of coordinates
/param[in] sum_sq the sum of squares
/param[in] n_data number of points
/param[in] ctrInd center index
*/
void KMCentersNode::post_neighbor(
   KMPointArray *sums, KMPoint *sum_sqs,std::vector<int> *weights,
   int center_ind) {
  IMP_assert((unsigned int)center_ind<sums->size(),
             "the center index is out of range\n");
  // increment sum
  for (int d = 0; d < centers_->get_dim(); d++) {
    (*((*sums)[center_ind]))[d] += sum_[d];
  }
  //incremet weight
  IMP_assert((unsigned int)center_ind<weights->size(),
              "the center index is out of range\n");
  (*weights)[center_ind] += n_data_;
  //increment sum of squares
  IMP_assert((unsigned int)center_ind<sum_sqs->size(),
             "the center index is out of range\n");
  (*sum_sqs)[center_ind] += sum_sq_;
}

int KMCentersNode::mid_center(const std::vector<int> &cands) {
  KMPoint *lo,*hi,mid;
  lo = bnd_box_.get_point(0);
  hi = bnd_box_.get_point(1);
  //get the mid point of the bounding box
  for (int d = 0; d < bnd_box_.get_dim(); d++) {
    mid.push_back(((*lo)[d]+(*hi)[d])/2);
  }
  //the closest center will be the closest to the middle of the bounding box
  double min_dist = km_distance2(mid,*((*centers_)[cands[0]]));
  int min_ind=0;
  for (unsigned int j = 1; j < cands.size(); j++) {
    double dist = km_distance2(mid,*((*centers_)[cands[j]]));
    if (dist < min_dist) {
      min_dist = dist;min_ind=j;
    }
  }
  return cands[min_ind];
}
IMPDOMINO_END_NAMESPACE

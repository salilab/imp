/**
 * \file KMFilterCenters.h
 * \brief Provides efficient algorithm for compuing distortions
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_KM_FILTER_CENTERS_H
#define IMPDOMINO_KM_FILTER_CENTERS_H

#include "KMCenters.h"
#include <iostream>
#include "random_generator.h"
#include "KMData.h"
#include "KMCentersTree.h"
#include <limits.h>
IMPDOMINO_BEGIN_NAMESPACE

//! Provides efficient algorithm for computing distortions, by a
//! filtering algorithm.
class IMPDOMINOEXPORT KMFilterCenters : public KMCenters{
public:
  KMFilterCenters(){}
  //! Constructor
  /**
  /param[in] k     number of centers
  /param[in] data  the data points
  /param[in] ini_cen_arr initial centers
  /param[in] df    damp factor
   */
  KMFilterCenters(int k, KMData* data,KMPointArray *ini_cen_arr=NULL,
                  double df = 1);
  //! Copy constructor
  /**
  /note the tree data is not deep copied, we just copy the pointer
   */
  KMFilterCenters(const KMFilterCenters &other);
  KMFilterCenters& operator=(const KMFilterCenters &other);
  virtual ~KMFilterCenters();
public:
  //! Returns sums
  KMPointArray *get_sums(bool auto_update = true){
    if (auto_update && !valid_) compute_distortion();
    return sums_;
  }
  // Returns sums of squares
  std::vector<double>* get_sum_sqs(bool auto_update = true){
    if (auto_update && !valid_) compute_distortion();
    return &sum_sqs_;
  }
  //! Return weights
  std::vector<int>* get_weights(bool auto_update = true){
    if (auto_update && !valid_) compute_distortion();
    return &weights_;
  }
  //! Returns total distortion
  double get_distortion(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return curr_dist_;
  }
  //! Returns average distortion
  double get_average_distortion(bool auto_update = true){
    if (auto_update && !valid_) compute_distortion();
    return curr_dist_/double(get_number_of_points());
  }
  //! Returns individual distortions
  std::vector<double>* get_distortions(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return &dists_;
  }
  //! Compute the assignment of points to centers
  /**  The request is calculated by the tree
  Even though this makes a full traversal of the kc-tree, it does not update
  the sum or sum of squares, etc., but it does not modify them either.
  Thus, we do not change the validation status.
  /param[in] close_center will contain the closest center to each data point
  */
  void get_assignments(std::vector<int> &close_center);

//! Generate random centers
virtual void generate_random_centers(int k) {
  std::cout<<"in generate_random"<<std::endl;
  if (ini_cen_arr_ != NULL) {
    std::cout<<"with initial points"<<std::endl;
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
    std::cout<<"without initial points"<<std::endl;
    data_points_->sample_centers(centers_,k,false);
  }
  invalidate();
}
void show(std::ostream& out=std::cout) const;
  //!  move centers to cluster centroids
  /** Moves each center point to the centroid of its associated cluster.
      We call compute_distortion() if necessary to compute the weights
      and sums.  The centroid is the weighted average of the sum of neighbors.
      Thus the ctrs[j] = sums[j] / weights[j].
      We generally allow a dampening factor on the motion, which is a floating
      quantity between 0 (full dampening) and 1 (no dampening).
      Given the dampening factor df, the above formula is:
      ctrs[j] = (1-df) * ctrs[j] + df * sums[j]/ weights[j]
  */
  void move_to_centroid();
protected:
//!  Compute distortions
/**  A distortion of a set of points from a set of centers is defined as the
the sum of squared distances from each point to its closest center.
To accelerate this calculation we make use of intermediate terms, such that
the distortion for center cj is:
dists[j] = SUM_i{ (pi - cj)^2}
         = SUM_i{ (pi^2 - 2*cj*pi + cj^2)}
         = SUM_i{ pi^2} - 2*cj*SUM_i{pi} + weights_[j]*cj^2
         = sum_sqs[j] - 2*(c[j].sums_[j]) + weights[j]*(cj^2)
Thus the individual distortion can be computed from these quantities.
The total distortion is the sum of the individual distortions.
*/
  void compute_distortion();
  void validate(){ valid_ = true; }
  //! Make invalid
  /** The function should be called after center update
   */
  void invalidate() {
    show();
    valid_ = false;
  }
protected:
  KMPointArray *sums_;// vector sum of points
  KMPoint sum_sqs_;// sum of squares
  std::vector<int> weights_; //number of data points assigned to each point
  KMPointArray *ini_cen_arr_; //initial guess of centers
  KMPoint dists_;// individual distortions
  double curr_dist_;// current total distortion
  bool valid_; // are sums/distortions valid?
  double damp_factor_; // dampening factor [0,1] - determines how much to
                       //consider old centers in move_to_centroid function
  KMCentersTree* tree_; //the centers tree of the data points
};
IMPDOMINO_END_NAMESPACE
#endif /* IMPDOMINO_KM_FILTER_CENTERS_H */

/**
 * \file KMFilterCenters.h
 * \brief Provides efficient algorithm for compuing distortions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_FILTER_CENTERS_H
#define IMPSTATISTICS_INTERNAL_KM_FILTER_CENTERS_H

#include "KMCenters.h"
#include <iostream>
#include "KMData.h"
#include "KMCentersTree.h"
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

//! Provides efficient algorithm for computing distortions, by a
//! filtering algorithm.
/**
   \unstable{KMFilterCenters}
*/
class IMPSTATISTICSEXPORT KMFilterCenters : public KMCenters {
 public:
  KMFilterCenters();
  //! Constructor
  /**
  \param[in] k     number of centers
  \param[in] data  the data points
  \param[in] ini_cen_arr initial centers
  \param[in] df    damp factor
   */
  KMFilterCenters(int k, KMData *data, KMPointArray *ini_cen_arr = nullptr,
                  double df = 1);
  virtual ~KMFilterCenters();

 public:
  //! Returns sums
  KMPointArray *get_sums(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return sums_;
  }
  // Returns sums of squares
  Floats *get_sum_sqs(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return &sum_sqs_;
  }
  //! Return weights
  Ints *get_weights(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return &weights_;
  }
  //! Returns total distortion
  double get_distortion(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return curr_dist_;
  }
  //! Returns average distortion
  double get_average_distortion(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return curr_dist_ / double(get_number_of_points());
  }
  //! Returns individual distortions
  Floats *get_distortions(bool auto_update = true) {
    if (auto_update && !valid_) compute_distortion();
    return &dists_;
  }
  //! Compute the assignment of points to centers
  /**  The request is calculated by the tree
  Even though this makes a full traversal of the kc-tree, it does not update
  the sum or sum of squares, etc., but it does not modify them either.
  Thus, we do not change the validation status.
  \param[in] close_center will contain the closest center to each data point
  */
  void get_assignments(Ints &close_center);

  //! Generate random centers
  virtual void generate_random_centers(int k);
  void show(std::ostream &out = std::cout) const;
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
  void clear_data();
  //!  Compute distortions
  /**  A distortion of a set of points from a set of centers is defined as the
  the sum of squared distances from each point to its closest center.
  To accelerate this calculation we make use of intermediate terms, such that
  the distortion for center cj to its closest points {pi} is:
  dists[j] = SUM_i{ (pi - cj)^2}
         = SUM_i{ (pi^2 - 2*cj*pi + cj^2)}
         = SUM_i{ pi^2} - 2*cj*SUM_i{pi} + weights_[j]*cj^2
         = sum_sqs[j] - 2*(c[j].sums_[j]) + weights[j]*(cj^2)
  Thus the individual distortion can be computed from these quantities.
  The total distortion is the sum of the individual distortions.
  */
  void compute_distortion();
  void validate() { valid_ = true; }
  //! Make invalid
  /** The function should be called after center update
   */
  void invalidate();

 protected:
  KMPointArray *sums_;         // vector sum of points
  KMPoint sum_sqs_;            // sum of squares
  Ints weights_;               //number of data points assigned to each point
  KMPointArray *ini_cen_arr_;  //initial guess of centers
  KMPoint dists_;              // individual distortions
  double curr_dist_;           // current total distortion
  bool valid_;                 // are sums/distortions valid?
  double damp_factor_;   // dampening factor [0,1] - determines how much to
                         //consider old centers in move_to_centroid function
  KMCentersTree *tree_;  //the centers tree of the data points
};

class IMPSTATISTICSEXPORT KMFilterCentersResults : public KMCenters {
 public:
  KMFilterCentersResults() {}
  ;
  //! Constructor
  /**
  \param[in] k     number of centers
  \param[in] data  the data points
  \param[in] ini_cen_arr initial centers
  \param[in] df    damp factor
   */
  KMFilterCentersResults(KMFilterCenters &full) : KMCenters(full) {
    close_center_.clear();
    full.get_assignments(close_center_);
    sums_ = new KMPointArray();
    copy_points(full.get_sums(), sums_);
    copy_point(full.get_sum_sqs(), &sum_sqs_);
    Ints *w = full.get_weights();
    weights_.clear();
    for (unsigned int i = 0; i < w->size(); i++) {
      weights_.push_back((*w)[i]);
    }
    copy_point(full.get_distortions(), &dists_);
    curr_dist_ = full.get_distortion();
  }
  KMFilterCentersResults &operator=(const KMFilterCentersResults &other) {
    if (this != &other) {  // avoid self assignment (x=x)
      KMCenters::operator=(other);
      close_center_.clear();
      for (unsigned int i = 0; i < other.close_center_.size(); i++) {
        close_center_.push_back(other.close_center_[i]);
      }
      sums_ = new KMPointArray();
      copy_points(other.sums_, sums_);
      copy_point(&other.sum_sqs_, &sum_sqs_);
      weights_.clear();
      for (unsigned int i = 0; i < other.weights_.size(); i++) {
        weights_.push_back(other.weights_[i]);
      }
      copy_point(&other.dists_, &dists_);
      curr_dist_ = other.curr_dist_;
    }
    return *this;
  }
  KMFilterCentersResults(const KMFilterCentersResults &other)
      : KMCenters(other) {
    close_center_.clear();
    for (unsigned int i = 0; i < other.close_center_.size(); i++) {
      close_center_.push_back(other.close_center_[i]);
    }
    sums_ = new KMPointArray();
    copy_points(other.sums_, sums_);
    copy_point(&other.sum_sqs_, &sum_sqs_);
    weights_.clear();
    for (unsigned int i = 0; i < other.weights_.size(); i++) {
      weights_.push_back(other.weights_[i]);
    }
    copy_point(&other.dists_, &dists_);
    curr_dist_ = other.curr_dist_;
  }
  ~KMFilterCentersResults() { deallocate_points(sums_); }

 public:
  //! Returns sums
  KMPointArray *get_sums() const { return sums_; }
  // Returns sums of squares
  const Floats *get_sum_sqs() const { return &sum_sqs_; }
  //! Return weights
  const Ints *get_weights() const { return &weights_; }
  //! Returns total distortion
  double get_distortion() const { return curr_dist_; }
  //! Returns average distortion
  double get_average_distortion() const {
    return curr_dist_ / double(get_number_of_points());
  }
  //! Returns individual distortions
  const Floats *get_distortions() const { return &dists_; }
  //! Get the assignment of points to centers
  const Ints *get_assignments() const { return &close_center_; }

  void show(std::ostream &out = std::cout) const {
    for (int j = 0; j < get_number_of_centers(); j++) {
      out << "    " << std::setw(4) << j << "\t";
      print_point(*((*centers_)[j]), out);
      out << " dist = " << std::setw(8) << dists_[j]
          << " weight = " << std::setw(8) << weights_[j] << std::endl;
    }
  }

 protected:
  KMPointArray *sums_;  // vector sum of points
  KMPoint sum_sqs_;     // sum of squares
  Ints weights_;        //number of data points assigned to each point
  KMPoint dists_;       // individual distortions
  double curr_dist_;    // current total distortion
  Ints close_center_;
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_FILTER_CENTERS_H */

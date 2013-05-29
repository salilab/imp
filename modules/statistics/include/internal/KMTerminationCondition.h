/**
 *  \file KMTerminationCondition.h
 *  \brief Stores data controlling the termination and phase changes
 *          of the kmean clustering
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_TERMINATION_CONDITION_H
#define IMPSTATISTICS_INTERNAL_KM_TERMINATION_CONDITION_H
#include <IMP/statistics/statistics_config.h>
#include <IMP/base_types.h>

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

//! Stores data controlling the termination and phase changes of
//! the kmean clustering
/**
   \unstable{KMTerminationCondition}
 */
class KMTerminationCondition {
 public:
  KMTerminationCondition() {
    abs_max_num_of_stages_ = 200;
    max_num_of_stages_for_run_ = 20;
    min_consecutive_rdl_ = 0.1;
    min_accumulated_rdl_ = 0.1;
  }
  KMTerminationCondition(Int abs_max_num_of_stages,
                         Int max_num_of_stages_for_run,
                         double min_consecutive_rdl,
                         double min_accumulated_rdl) {
    abs_max_num_of_stages_ = abs_max_num_of_stages;
    max_num_of_stages_for_run_ = max_num_of_stages_for_run;
    min_consecutive_rdl_ = min_consecutive_rdl;
    min_accumulated_rdl_ = min_accumulated_rdl;
  }
  inline void set_abs_max_num_of_stages(Int a) { abs_max_num_of_stages_ = a; }
  inline Int get_abs_max_num_of_stages() const {
    return abs_max_num_of_stages_;
  }
  inline void set_max_num_of_stages_for_run(Int a) {
    max_num_of_stages_for_run_ = a;
  }
  inline Int get_max_num_of_stages_for_run() const {
    return max_num_of_stages_for_run_;
  }
  inline void set_min_consecutive_rdl(double a) { min_consecutive_rdl_ = a; }
  inline double get_min_consecutive_rdl() const { return min_consecutive_rdl_; }
  inline void set_min_accumulated_rdl(double a) { min_accumulated_rdl_ = a; }
  inline double get_min_accumulated_rdl() const { return min_accumulated_rdl_; }

 protected:
  Int abs_max_num_of_stages_;  //max number of stages it total (for all runs)
  Int max_num_of_stages_for_run_;  //max number of stages for a single run
  //min value for consecutive relative distortion loss
  double min_consecutive_rdl_;
  //min value for accumulated  relative distortion loss within a single run
  double min_accumulated_rdl_;
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_TERMINATION_CONDITION_H */

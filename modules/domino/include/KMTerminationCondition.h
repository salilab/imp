/**
 *  \file KMTerminationCondition.h
 *  \brief Stores data controlling the termination and phase changes
 *          of the kmean clustering
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_KM_TERMINATION_CONDITION_H
#define IMPDOMINO_KM_TERMINATION_CONDITION_H
IMPDOMINO_BEGIN_NAMESPACE
//! Stores data controlling the termination and phase changes of
//! the kmean clustering
class KMTerminationCondition {
public:
  KMTerminationCondition() {
    abs_max_num_of_stages_ = 200;
    max_num_of_stages_for_run_ = 20;
    min_consecutive_rdl_ = 0.1;
    min_accumulated_rdl_ = 0.1;
  }
  KMTerminationCondition(Int abs_max_num_of_stages,
    Int max_num_of_stages_for_run,double min_consecutive_rdl,
    double min_accumulated_rdl) {
    abs_max_num_of_stages_ = abs_max_num_of_stages;
    max_num_of_stages_for_run_ = max_num_of_stages_for_run;
    min_consecutive_rdl_ = min_consecutive_rdl;
    min_accumulated_rdl_ = min_accumulated_rdl;
  }
  inline void set_abs_max_num_of_stages(Int a) {abs_max_num_of_stages_ = a;}
  inline Int get_abs_max_num_of_stages() const {return abs_max_num_of_stages_;}
  inline void set_max_num_of_stages_for_run(Int a) {
         max_num_of_stages_for_run_ = a;}
  inline Int get_max_num_of_stages_for_run() const {
         return max_num_of_stages_for_run_;}
  inline void set_min_consecutive_rdl(double a) {min_consecutive_rdl_=a;}
  inline double get_min_consecutive_rdl() const {return min_consecutive_rdl_;}
  inline void set_min_accumulated_rdl(double a) {min_accumulated_rdl_=a;}
  inline double get_min_accumulated_rdl() const {return min_accumulated_rdl_;}

protected:
  Int  abs_max_num_of_stages_; //max number of stages it total (for all runs)
  Int  max_num_of_stages_for_run_;//max number of stages for a single run
  //min value for consecutive relative distortion loss
  double min_consecutive_rdl_;
  //min value for accumulated  relative distortion loss within a single run
  double min_accumulated_rdl_;
};
IMPDOMINO_END_NAMESPACE
#endif  /* IMPDOMINO_KM_TERMINATION_CONDITION_H */

/**
 *  \file KMLocalSearchLloyd.cpp
 *  \brief Lloyd's algorithm with random restarts
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/KMLocalSearchLloyd.h>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
void KMLocalSearchLloyd::reset() {
  KMLocalSearch::reset();
  is_new_trail_ = false;
  init_trail_dist_ = curr_->get_distortion();
  IMP_LOG_WRITE(VERBOSE,log_stage());
}
void KMLocalSearchLloyd::log_stage(std::ostream &out) {
  out<<"\t<stage: "<< stage_num_
     << " curr: "<< curr_->get_average_distortion()
     <<" best: " << best_.get_average_distortion()
     <<" accum_rdl: " << get_accumulated_rdl()*100 << "% >" << std::endl;
  best_.show();
}
void KMLocalSearchLloyd::end_stage() {
  IMP_LOG_VERBOSE("end Lloyd stage\n");
  KMLocalSearch::end_stage();
  // get distortions
  if (curr_->get_average_distortion() < best_.get_average_distortion()) {
    best_ = KMFilterCentersResults(*curr_);
  }
  IMP_LOG_WRITE(VERBOSE,log_stage());
  IMP_LOG_VERBOSE("end Lloyd stage==\n");
}
bool KMLocalSearchLloyd::is_run_done() {
  // check if we already preformed too many stages
  if (KMLocalSearch::is_run_done() ||
  (stage_num_ - run_init_stage_ >= term_->get_max_num_of_stages_for_run())) {
      return true;
  }
  // check if the run is just starting
  else if (is_new_trail_) {
    is_new_trail_ = false;
    init_trail_dist_ = curr_->get_distortion();
    return false;
  }
  // continue if the run if there is an improvement
  return get_accumulated_rdl() >= term_->get_min_accumulated_rdl();
}
void KMLocalSearchLloyd::end_run() {
  // check if the run was unsuccessful
  if (get_accumulated_rdl() < term_->get_min_accumulated_rdl()) {
    is_new_trail_ = true;
  }
  else {
    init_trail_dist_ = curr_->get_distortion();
  }
  log_run();
}
void KMLocalSearchLloyd::preform_stage() {
  // if this is the first stage of the run, randomly select centers
  if ( run_init_stage_ == stage_num_) {
    IMP_LOG_VERBOSE(
    "KMLocalSearchLloyd::preform_stage generate random centers \n");
    curr_->generate_random_centers(num_of_centers_);
  }
  else {
    IMP_LOG_VERBOSE(
    "KMLocalSearchLloyd::preform_stage move to centroids \n");
    curr_->move_to_centroid();
  }
}
IMPSTATISTICS_END_INTERNAL_NAMESPACE

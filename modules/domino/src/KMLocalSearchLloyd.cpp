/**
 *  \file KMLocalSearchLloyd.cpp
 *  \brief Lloyd's algorithm with random restarts
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/domino/KMLocalSearchLloyd.h>
IMPDOMINO_BEGIN_NAMESPACE
void KMLocalSearchLloyd::reset() {
  KMLocalSearch::reset();
  is_new_trail_ = false;
  init_trail_dist_ = curr_->get_distortion();
  log_stage();
}
void KMLocalSearchLloyd::log_stage() {
  IMP_LOG(VERBOSE,"\t<stage: "<< stage_num_
          << " curr: "<< curr_->get_average_distortion()
          <<" best: " << best_.get_average_distortion()
          <<" accum_rdl: " << get_accumulated_rdl()*100 << "% >" << std::endl);
}
void KMLocalSearchLloyd::end_stage() {
  KMLocalSearch::end_stage();
  // get distortions
  if (curr_->get_average_distortion() < best_.get_average_distortion()) {
    best_ = *curr_;
  }
  log_stage();
}
bool KMLocalSearchLloyd::is_run_done() {
  // check if we already preformed too many stages
  if (KMLocalSearch::is_run_done() or
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
    curr_->generate_random_centers(num_of_centers_);
  }
  else {
    curr_->move_to_centroid();
  }
}
IMPDOMINO_END_NAMESPACE

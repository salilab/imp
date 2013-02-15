/**
 *  \file KMLocalSearch.cpp   \brief Generic algorithm from k-means
 *                                 clustering by local search
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/statistics/internal/KMLocalSearch.h>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
KMLocalSearch::KMLocalSearch(KMFilterCenters *sol,
  KMTerminationCondition *term) {
  curr_ = sol;
  curr_->get_distortion(); // so that best_ will have a valid distortion value
  best_ = KMFilterCentersResults(*sol);
  term_ = term;
  num_of_data_points_ = curr_->get_number_of_points();
  num_of_centers_ = curr_->get_number_of_centers();
  dim_ = curr_->get_dim();
  stage_num_ = 0;
  run_init_stage_= 0;
  //  maxTotStage = term.getMaxTotStage(kCtrs, nPts); //TODO - return ?
}
void KMLocalSearch::execute()
{
  int i,j; i=0;j=0;
  reset();
  while (!is_done()) {
    IMP_LOG_VERBOSE("KMLocalSearch::execute run: " << i <<"\n");
    begin_run();
    j=0;
    do {
      IMP_LOG_VERBOSE("KMLocalSearch::execute stage: " <<j <<"\n");
      begin_stage();
      preform_stage();
      end_stage();
      j++;
    } while (!is_run_done());
    end_run();
    i++;
    IMP_LOG_VERBOSE("KMLocalSearch::execute end run: " <<i <<"\n");
    try_acceptance();
  }
}
void KMLocalSearch::reset() {
  stage_num_ = 0;
  run_init_stage_ = 0;
  curr_->generate_random_centers(num_of_centers_);
  curr_->get_distortion();
  best_ = KMFilterCentersResults(*curr_);
}
bool KMLocalSearch::is_done() const {
  return stage_num_ >= term_->get_abs_max_num_of_stages();
}
void KMLocalSearch::begin_run() {
  run_init_stage_ = stage_num_;
}
void KMLocalSearch::end_stage() {
  stage_num_++;
}
void KMLocalSearch::try_acceptance() {
  // is current distortion lower?
  IMP_LOG_VERBOSE("KMLocalSearch::try_acceptance for"
    << " old distortions=" << best_.get_distortion()
    << " new distortions=" << curr_->get_distortion() <<"\n");
  IMP_LOG_VERBOSE("The current filtered centers are :\n");
  IMP_LOG_WRITE(VERBOSE,curr_->show(IMP_STREAM));
  if (curr_->get_distortion() <= best_.get_distortion()) {
    IMP_LOG_VERBOSE("KMLocalSearch::try_acceptance new centers accepted.\n");
    best_ = KMFilterCentersResults(*curr_);
  }
}
void KMLocalSearch::log_stage(std::ostream &out) {
  out<<"\t<stage: " << stage_num_
     << " curr: " << curr_->get_average_distortion()
     <<" best: " <<  best_.get_average_distortion()
     << " > " << std::endl;
  best_.show(out);
}
IMPSTATISTICS_END_INTERNAL_NAMESPACE

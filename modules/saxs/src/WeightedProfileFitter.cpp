/**
 * \file WeightedProfileFitter.cpp
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/WeightedProfileFitter.h>

#include <IMP/saxs/internal/nnls.h>

IMPSAXS_BEGIN_NAMESPACE

std::vector<double> WeightedProfileFitter::empty_weights_;

WeightedProfileFitter::WeightedProfileFitter(const Profile& exp_profile):
  IMP::saxs::ProfileFitter<ChiScore>(exp_profile),
  W_(exp_profile.size()), Wb_(exp_profile.size()), A_(exp_profile.size(), 2) {

  for(unsigned int i=0; i<exp_profile_.size(); i++) {
    Wb_[i] = exp_profile_.get_intensity(i);
    W_[i] = 1.0/(exp_profile_.get_error(i)*exp_profile_.get_error(i));
  }
  Wb_ = W_ * Wb_;
}

Float WeightedProfileFitter::compute_score(
                         const std::vector<IMP::saxs::Profile *>& profiles,
                         std::vector<double>& weights) const {

  // no need to compute weighted profile for ensemble of size 1
  if(profiles.size() == 1)
    return scoring_function_->compute_score(exp_profile_, *profiles[0]);

  //compute_weights(profiles, weights);
  int m = profiles.size();
  int n = exp_profile_.size();

  WeightedProfileFitter* non_const_this =
    const_cast<WeightedProfileFitter*>(this);
  if(A_.dim2() != m) non_const_this->A_.resize(n, m);

  for(int j=0; j<m; j++) {
    for(int i=0; i<n; i++) {
      (non_const_this->A_)[i][j] = profiles[j]->get_intensity(i);
    }
  }

  // compute weights
  internal::Vector w = autoregn(W_*A_, Wb_);
  // normalize weights so they sum up to 1.0
  w /= w.sum();

  // computed weighted profile
  IMP::saxs::Profile weighted_profile(exp_profile_.get_min_q(),
                                      exp_profile_.get_max_q(),
                                      exp_profile_.get_delta_q());
  internal::Vector wp = A_*w;

  for(unsigned int k=0; k<profiles[0]->size(); k++)
    weighted_profile.add_entry(profiles[0]->get_q(k), wp[k]);

  weights.resize(w.size()); for(int i=0; i<w.size(); i++) weights[i] = w[i];
  return scoring_function_->compute_score(exp_profile_, weighted_profile);
}

FitParameters WeightedProfileFitter::fit_profile(
                            std::vector<IMP::saxs::Profile *>& partial_profiles,
                            float min_c1, float max_c1,
                            float min_c2, float max_c2,
                            std::vector<double>& weights,
                            const std::string fit_file_name) const {

  FitParameters fp = search_fit_parameters(partial_profiles,
                                           min_c1, max_c1, min_c2, max_c2,
                                           std::numeric_limits<float>::max(),
                                           weights);

  if(fit_file_name.size() > 0) {
    float best_c1 = fp.get_c1();
    float best_c2 = fp.get_c2();

    // compute a profile for best c1/c2 combination
    for(unsigned int i=0; i<partial_profiles.size(); i++)
      partial_profiles[i]->sum_partial_profiles(best_c1, best_c2);

    // computed weighted profile
    IMP::saxs::Profile weighted_profile(exp_profile_.get_min_q(),
                                        exp_profile_.get_max_q(),
                                        exp_profile_.get_delta_q());
    for(unsigned int i=0; i<weights.size(); i++)
      weighted_profile.add(*partial_profiles[i], weights[i]);

    // compute scale
    Float c = scoring_function_->compute_scale_factor(exp_profile_,
                                                      weighted_profile);

    ProfileFitter<ChiScore>::write_SAXS_fit_file(fit_file_name,
                                                 weighted_profile,
                                                 fp.get_chi(), c);
  }
  return fp;
}


FitParameters WeightedProfileFitter::search_fit_parameters(
                           std::vector<IMP::saxs::Profile *>& partial_profiles,
                           float min_c1, float max_c1,
                           float min_c2, float max_c2,
                           float old_chi, std::vector<double>& weights) const {
  int c1_cells = 10;
  int c2_cells = 10;
  if(old_chi < (std::numeric_limits<float>::max()-1)) { // second iteration
    c1_cells = 5;
    c2_cells = 5;
  }

  float delta_c1 = (max_c1-min_c1)/c1_cells;
  float delta_c2 = (max_c2-min_c2)/c2_cells;

  bool last_c1 = false;
  bool last_c2 = false;
  if(delta_c1 < 0.0001) { c1_cells=1; delta_c1 = max_c1-min_c1; last_c1=true; }
  if(delta_c2 < 0.001) { c2_cells=1; delta_c2 = max_c2-min_c2; last_c2=true; }
  float best_c1(1.0), best_c2(0.0), best_chi(old_chi);
  bool best_set = false;

  // c1 iteration
  float c1(min_c1);
  for(int i=0; i<=c1_cells; i++, c1+= delta_c1) {
    // c2 iteration
    float c2 = min_c2;
    for(int j=0; j<=c2_cells; j++, c2+= delta_c2) {
      // sum up the profiles for c1/c2 combo
      for(unsigned int k=0; k<partial_profiles.size(); k++)
        partial_profiles[k]->sum_partial_profiles(c1, c2);
      std::vector<double> curr_weights;
      float curr_chi = compute_score(partial_profiles, curr_weights);
      if(!best_set || curr_chi < best_chi) {
        best_set = true;
        best_chi = curr_chi;
        best_c1 = c1;
        best_c2 = c2;
        weights = curr_weights;
      }
    }
  }

  if(std::fabs(best_chi-old_chi) > 0.005 &&
     (!(last_c1 && last_c2))) { //refine more
    min_c1 = std::max(best_c1-delta_c1, min_c1);
    max_c1 = std::min(best_c1+delta_c1, max_c1);
    min_c2 = std::max(best_c2-delta_c2, min_c2);
    max_c2 = std::min(best_c2+delta_c2, max_c2);
    return search_fit_parameters(partial_profiles,
                                 min_c1, max_c1, min_c2, max_c2,
                                 best_chi, weights);
  }
  return FitParameters(best_chi, best_c1, best_c2);
}

IMPSAXS_END_NAMESPACE

/**
 * \file IMP/saxs/WeightedProfileFitter.h
 * \brief  Fitting of multiple profiles to the experimental one.
 * The weights of the profiles are computed analytically using
 * non-negative least squares fitting (NNLS)
 *
 * \authors Dina Schneidman
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_WEIGHTED_PROFILE_FITTER_H
#define IMPSAXS_WEIGHTED_PROFILE_FITTER_H

#include "ProfileFitter.h"
#include "ChiScore.h"
#include "WeightedFitParameters.h"
#include "nnls.h"
#include <Eigen/Dense>

IMPSAXS_BEGIN_NAMESPACE

//! Fitting of multiple profiles to the experimental one.
/** The weights of the profiles are computed analytically using
    non-negative least squares fitting (NNLS).
*/
template <typename ScoringFunctionT = ChiScore>
class WeightedProfileFitter : public ProfileFitter<ScoringFunctionT> {

 public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
  */
  WeightedProfileFitter(const Profile* exp_profile) :
    ProfileFitter<ScoringFunctionT>(exp_profile),
    W_(exp_profile->size(), 1),
    Wb_(exp_profile->size()),
    A_(exp_profile->size(), 2) {

    Eigen::VectorXf b(exp_profile->size());
    for (unsigned int i = 0; i < exp_profile->size(); i++) {
      Wb_(i) = exp_profile->get_intensity(i);
      W_(i) = 1.0 / (exp_profile->get_error(i));
    }
    Wb_ = W_.asDiagonal() * Wb_;
  }

  //! compute a weighted score that minimizes chi
  /**
     it is assumed that the q values of the profiles are the same as
     the q values of the experimental profile. Use Profile::resample to resample
     if(NNLS = true, solve non-negative least squares, otherwise solve just
     least squares, that may return negative weights to be discarded later
  */
  double compute_score(const ProfilesTemp& profiles, Vector<double>& weights,
                       bool use_offset = false, bool NNLS = true) const;

  //! fit profiles by optimization of c1/c2 and weights
  /**
     it is assumed that the q values of the profiles are the same as
     the q values of the experimental profile. Use Profile::resample to resample
  */
  WeightedFitParameters fit_profile(ProfilesTemp partial_profiles,
                                    double min_c1 = 0.95, double max_c1 = 1.05,
                                    double min_c2 = -2.0, double max_c2 = 4.0,
                                    bool use_offset = false) const;

  //! write a fit file
  void write_fit_file(ProfilesTemp partial_profiles,
                      const WeightedFitParameters& fp,
                      const std::string fit_file_name,
                      bool use_offset = false) const;

 private:
  WeightedFitParameters search_fit_parameters(
      ProfilesTemp& partial_profiles, double min_c1, double max_c1, double min_c2,
      double max_c2, double old_chi, Vector<double>& weights, bool use_offset) const;

 private:
  Eigen::MatrixXf W_;  // weights matrix

  // weights matrix multiplied by experimental intensities vector
  Eigen::VectorXf Wb_;

  // intensities
  Eigen::MatrixXf A_;
};

template <typename ScoringFunctionT>
double WeightedProfileFitter<ScoringFunctionT>::compute_score(
                                            const ProfilesTemp& profiles,
                                            Vector<double>& weights,
                                            bool use_offset, bool nnls) const {

  // no need to compute weighted profile for ensemble of size 1
  if (profiles.size() == 1) {
    weights.resize(1);
    weights[0] = 1.0;
    return  ProfileFitter<ScoringFunctionT>::scoring_function_->compute_score(
                                       ProfileFitter<ScoringFunctionT>::exp_profile_,
                                       profiles[0], use_offset);
  }

  // compute_weights(profiles, weights);
  int m = profiles.size();
  int n = ProfileFitter<ScoringFunctionT>::exp_profile_->size();

  // fill in A_
  WeightedProfileFitter* non_const_this =
      const_cast<WeightedProfileFitter*>(this);

  if (A_.cols() != m) non_const_this->A_.resize(n, m);
  for (int j = 0; j < m; j++) {
    for (int i = 0; i < n; i++) {
      (non_const_this->A_)(i, j) = profiles[j]->get_intensity(i);
    }
  }

  Eigen::VectorXf w;
  if (!nnls) {  // solve least squares
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(W_.asDiagonal() * A_,
                                          Eigen::ComputeThinU
                                          | Eigen::ComputeThinV);
    w = svd.solve(Wb_);
    // zero the negatives
    for (int i = 0; i < w.size(); i++)
      if (w(i) < 0) w(i) = 0;
  } else {
    w = NNLS(W_.asDiagonal() * A_, Wb_);
  }
  w /= w.sum();

  // compute weighted profile
  IMP_NEW(Profile, weighted_profile,
          (ProfileFitter<ScoringFunctionT>::exp_profile_->get_min_q(),
           ProfileFitter<ScoringFunctionT>::exp_profile_->get_max_q(),
           ProfileFitter<ScoringFunctionT>::exp_profile_->get_delta_q()));
  Eigen::VectorXf wp = A_ * w;
  weighted_profile->set_qs(profiles[0]->get_qs());
  weighted_profile->set_intensities(wp);
  weights.resize(w.size());
  for (int i = 0; i < w.size(); i++) weights[i] = w[i];

  return ProfileFitter<ScoringFunctionT>::scoring_function_->compute_score(
                                   ProfileFitter<ScoringFunctionT>::exp_profile_,
                                   weighted_profile, use_offset);
}

template <typename ScoringFunctionT>
WeightedFitParameters WeightedProfileFitter<ScoringFunctionT>::fit_profile(
                                           ProfilesTemp partial_profiles,
                                           double min_c1, double max_c1,
                                           double min_c2, double max_c2,
                                           bool use_offset) const {
  Vector<double> weights;
  WeightedFitParameters fp =
      search_fit_parameters(partial_profiles, min_c1, max_c1, min_c2, max_c2,
                            std::numeric_limits<double>::max(), weights, use_offset);
  return fp;
}

template <typename ScoringFunctionT>
void WeightedProfileFitter<ScoringFunctionT>::write_fit_file(
                                         ProfilesTemp partial_profiles,
                                         const WeightedFitParameters& fp,
                                         const std::string fit_file_name,
                                         bool use_offset) const {
  double best_c1 = fp.get_c1();
  double best_c2 = fp.get_c2();

  // compute a profile for best c1/c2 combination
  for (unsigned int i = 0; i < partial_profiles.size(); i++)
    partial_profiles[i]->sum_partial_profiles(best_c1, best_c2);

  if (partial_profiles.size() == 1) {
    // compute scale
    double offset = 0.0;
    if (use_offset)
      offset =
        ProfileFitter<ScoringFunctionT>::scoring_function_->compute_offset(
                                   ProfileFitter<ScoringFunctionT>::exp_profile_,
                                   partial_profiles[0]);

    double c =
      ProfileFitter<ScoringFunctionT>::scoring_function_->compute_scale_factor(
                                   ProfileFitter<ScoringFunctionT>::exp_profile_,
                                   partial_profiles[0], offset);
    ProfileFitter<ScoringFunctionT>::write_SAXS_fit_file(
                                   fit_file_name, partial_profiles[0], fp.get_chi_square(), c, offset);
  } else {

    // computed weighted profile
    IMP_NEW(Profile, weighted_profile,
            (ProfileFitter<ScoringFunctionT>::exp_profile_->get_min_q(),
             ProfileFitter<ScoringFunctionT>::exp_profile_->get_max_q(),
             ProfileFitter<ScoringFunctionT>::exp_profile_->get_delta_q()));

    const Vector<double>& weights = fp.get_weights();
    for (unsigned int i = 0; i < weights.size(); i++)
      weighted_profile->add(partial_profiles[i], weights[i]);

    // compute scale
    double offset = 0.0;
    if (use_offset)
      offset =
        ProfileFitter<ScoringFunctionT>::scoring_function_->compute_offset(
                                   ProfileFitter<ScoringFunctionT>::exp_profile_,
                                   weighted_profile);
    double c =
      ProfileFitter<ScoringFunctionT>::scoring_function_->compute_scale_factor(
                                   ProfileFitter<ScoringFunctionT>::exp_profile_,
                                   weighted_profile, offset);

    ProfileFitter<ScoringFunctionT>::write_SAXS_fit_file(
                                   fit_file_name, weighted_profile,
                                   fp.get_chi_square(), c, offset);
  }
}

template <typename ScoringFunctionT>
WeightedFitParameters WeightedProfileFitter<ScoringFunctionT>::search_fit_parameters(
                                ProfilesTemp& partial_profiles,
                                double min_c1, double max_c1,
                                double min_c2, double max_c2,
                                double old_chi, Vector<double>& weights,
                                bool use_offset) const {
  int c1_cells = 10;
  int c2_cells = 10;
  if (old_chi < (std::numeric_limits<double>::max() - 1)) {  // second iteration
    c1_cells = 5;
    c2_cells = 5;
  }

  double delta_c1 = (max_c1 - min_c1) / c1_cells;
  double delta_c2 = (max_c2 - min_c2) / c2_cells;

  bool last_c1 = false;
  bool last_c2 = false;
  if (delta_c1 < 0.0001) {
    c1_cells = 1;
    delta_c1 = max_c1 - min_c1;
    last_c1 = true;
  }
  if (delta_c2 < 0.001) {
    c2_cells = 1;
    delta_c2 = max_c2 - min_c2;
    last_c2 = true;
  }
  double best_c1(1.0), best_c2(0.0), best_chi(old_chi);
  bool best_set = false;

  // c1 iteration
  double c1(min_c1);
  for (int i = 0; i <= c1_cells; i++, c1 += delta_c1) {
    // c2 iteration
    double c2 = min_c2;
    for (int j = 0; j <= c2_cells; j++, c2 += delta_c2) {
      // sum up the profiles for c1/c2 combo
      for (unsigned int k = 0; k < partial_profiles.size(); k++)
        partial_profiles[k]->sum_partial_profiles(c1, c2);
      Vector<double> curr_weights;
      double curr_chi = compute_score(partial_profiles, curr_weights, use_offset);
      if (!best_set || curr_chi < best_chi) {
        best_set = true;
        best_chi = curr_chi;
        best_c1 = c1;
        best_c2 = c2;
        weights = curr_weights;
      }
    }
  }

  if (std::fabs(best_chi - old_chi) > 0.005 &&
      (!(last_c1 && last_c2))) {  // refine more
    min_c1 = std::max(best_c1 - delta_c1, min_c1);
    max_c1 = std::min(best_c1 + delta_c1, max_c1);
    min_c2 = std::max(best_c2 - delta_c2, min_c2);
    max_c2 = std::min(best_c2 + delta_c2, max_c2);
    return search_fit_parameters(partial_profiles, min_c1, max_c1, min_c2,
                                 max_c2, best_chi, weights, use_offset);
  }
  return WeightedFitParameters(best_chi, best_c1, best_c2, weights);
}

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_WEIGHTED_PROFILE_FITTER_H */

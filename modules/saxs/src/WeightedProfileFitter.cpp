/**
 * \file WeightedProfileFitter.cpp
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/WeightedProfileFitter.h>

#include <Eigen/Dense>

using namespace Eigen;

IMPSAXS_BEGIN_NAMESPACE

namespace {

Eigen::VectorXf NNLS(const Eigen::MatrixXf& A, const Eigen::VectorXf& b) {

  // TODO: make JacobiSVD a class object to avoid memory re-allocations
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(A, ComputeThinU | ComputeThinV);
  Eigen::VectorXf x = svd.solve(b);

  // compute a small negative tolerance
  float tol = 0;
  int n = A.cols();
  int m = A.rows();

  // count initial negatives
  int negs = 0;
  for (int i = 0; i < n; i++)
    if (x[i] < 0.0) negs++;
  if (negs <= 0) return x;

  int sip = int(negs / 100);
  if (sip < 1) sip = 1;

  Eigen::VectorXf zeroed = Eigen::VectorXf::Zero(n);
  Eigen::MatrixXf C = A;

  // iteratively zero some x values
  for (int count = 0; count < n; count++) {  // loop till no negatives found
    // count negatives and choose how many to treat
    negs = 0;
    for (int i = 0; i < n; i++)
      if (zeroed[i] < 1.0 && x[i] < tol) negs++;
    if (negs <= 0) break;

    int gulp = std::max(negs / 20, sip);

    // zero the most negative solution values
    for (int k = 1; k <= gulp; k++) {
      int p = -1;
      float worst = 0.0;
      for (int j = 0; j < n; j++)
        if (zeroed[j] < 1.0 && x[j] < worst) {
          p = j;
          worst = x[p];
        }
      if (p < 0) break;
      for (int i = 0; i < m; i++) C(i, p) = 0.0;
      zeroed[p] = 9;
    }

    // re-solve
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, ComputeThinU | ComputeThinV);
    x = svd.solve(b);
  }

  for (int j = 0; j < n; j++)
    if (x[j] < 0.0 && std::abs(x[j]) <= std::abs(tol)) x[j] = 0.0;

  return x;
}
}

WeightedProfileFitter::WeightedProfileFitter(const Profile* exp_profile)
    : ProfileFitter<ChiScore>(exp_profile),
      W_(exp_profile->size(), 1),
      Wb_(exp_profile->size()),
      A_(exp_profile->size(), 2) {

  Eigen::VectorXf b(exp_profile->size());

  for (unsigned int i = 0; i < exp_profile_->size(); i++) {
    Wb_(i) = exp_profile_->get_intensity(i);
    W_(i) = 1.0 / (exp_profile_->get_error(i));
  }
  Wb_ = W_.asDiagonal() * Wb_;
}

Float WeightedProfileFitter::compute_score(const ProfilesTemp& profiles,
                                           std::vector<double>& weights,
                                           bool nnls) const {

  // no need to compute weighted profile for ensemble of size 1
  if (profiles.size() == 1) {
    weights.resize(1);
    weights[0] = 1.0;
    return scoring_function_->compute_score(exp_profile_, profiles[0]);
  }

  // compute_weights(profiles, weights);
  int m = profiles.size();
  int n = exp_profile_->size();

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
                                          ComputeThinU | ComputeThinV);
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
          (exp_profile_->get_min_q(), exp_profile_->get_max_q(),
           exp_profile_->get_delta_q()));
  Eigen::VectorXf wp = A_ * w;
  for (unsigned int k = 0; k < profiles[0]->size(); k++)
    weighted_profile->add_entry(profiles[0]->get_q(k), wp[k]);

  weights.resize(w.size());
  for (int i = 0; i < w.size(); i++) weights[i] = w[i];

  return scoring_function_->compute_score(exp_profile_, weighted_profile);
}

WeightedFitParameters WeightedProfileFitter::fit_profile(
    ProfilesTemp partial_profiles, float min_c1, float max_c1, float min_c2,
    float max_c2) const {
  std::vector<double> weights;
  WeightedFitParameters fp =
      search_fit_parameters(partial_profiles, min_c1, max_c1, min_c2, max_c2,
                            std::numeric_limits<float>::max(), weights);
  return fp;
}

void WeightedProfileFitter::write_fit_file(
    ProfilesTemp partial_profiles, const WeightedFitParameters& fp,
    const std::string fit_file_name) const {
  float best_c1 = fp.get_c1();
  float best_c2 = fp.get_c2();

  // compute a profile for best c1/c2 combination
  for (unsigned int i = 0; i < partial_profiles.size(); i++)
    partial_profiles[i]->sum_partial_profiles(best_c1, best_c2);

  if (partial_profiles.size() == 1) {
    // compute scale
    Float c = scoring_function_->compute_scale_factor(exp_profile_,
                                                      partial_profiles[0]);
    ProfileFitter<ChiScore>::write_SAXS_fit_file(
        fit_file_name, partial_profiles[0], fp.get_chi(), c);
  } else {

    // computed weighted profile
    IMP_NEW(IMP::saxs::Profile, weighted_profile,
            (exp_profile_->get_min_q(), exp_profile_->get_max_q(),
             exp_profile_->get_delta_q()));

    const std::vector<double>& weights = fp.get_weights();
    for (unsigned int i = 0; i < weights.size(); i++)
      weighted_profile->add(partial_profiles[i], weights[i]);

    // compute scale
    Float c =
        scoring_function_->compute_scale_factor(exp_profile_, weighted_profile);

    ProfileFitter<ChiScore>::write_SAXS_fit_file(
        fit_file_name, weighted_profile, fp.get_chi(), c);
  }
}

WeightedFitParameters WeightedProfileFitter::search_fit_parameters(
    ProfilesTemp& partial_profiles, float min_c1, float max_c1, float min_c2,
    float max_c2, float old_chi, std::vector<double>& weights) const {
  int c1_cells = 10;
  int c2_cells = 10;
  if (old_chi < (std::numeric_limits<float>::max() - 1)) {  // second iteration
    c1_cells = 5;
    c2_cells = 5;
  }

  float delta_c1 = (max_c1 - min_c1) / c1_cells;
  float delta_c2 = (max_c2 - min_c2) / c2_cells;

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
  float best_c1(1.0), best_c2(0.0), best_chi(old_chi);
  bool best_set = false;

  // c1 iteration
  float c1(min_c1);
  for (int i = 0; i <= c1_cells; i++, c1 += delta_c1) {
    // c2 iteration
    float c2 = min_c2;
    for (int j = 0; j <= c2_cells; j++, c2 += delta_c2) {
      // sum up the profiles for c1/c2 combo
      for (unsigned int k = 0; k < partial_profiles.size(); k++)
        partial_profiles[k]->sum_partial_profiles(c1, c2);
      std::vector<double> curr_weights;
      float curr_chi = compute_score(partial_profiles, curr_weights);
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
                                 max_c2, best_chi, weights);
  }
  return WeightedFitParameters(best_chi, best_c1, best_c2, (Floats)weights);
}

IMPSAXS_END_NAMESPACE

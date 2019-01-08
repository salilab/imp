/**
 *  \file ChiScore.h   \brief Basic SAXS scoring
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/ChiScore.h>

IMPSAXS_BEGIN_NAMESPACE

double ChiScore::compute_score(const Profile* exp_profile,
                               const Profile* model_profile,
                               bool use_offset) const {
  double offset = 0.0;
  if (use_offset) offset = compute_offset(exp_profile, model_profile);
  double c = compute_scale_factor(exp_profile, model_profile, offset);

  double chi_square = 0.0;
  unsigned int profile_size =
    std::min(model_profile->size(), exp_profile->size());

  const Eigen::VectorXf& errors = exp_profile->get_errors();
  const Eigen::VectorXf& exp_intensities = exp_profile->get_intensities();
  const Eigen::VectorXf& model_intensities = model_profile->get_intensities();

  Eigen::VectorXf delta = exp_intensities - c * model_intensities;
  if(use_offset) delta.array() += offset;

  for(int i=0; i<delta.size(); i++) {
    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta(i) / exp_intensities(i)) >= 1.0e-15)
      chi_square += square(delta(i)) / square(errors(i));
  }

  chi_square /= profile_size;
  return chi_square; //sqrt(chi_square);
}

double ChiScore::compute_scale_factor(const Profile* exp_profile,
                                      const Profile* model_profile,
                                      const double offset) const {

  const Eigen::VectorXf& errors = exp_profile->get_errors();
  const Eigen::VectorXf& exp_intensities = exp_profile->get_intensities();
  const Eigen::VectorXf& model_intensities = model_profile->get_intensities();

  Eigen::VectorXf square_errors = errors.cwiseProduct(errors);
  for (int k = 0; k < square_errors.size(); k++) {
    square_errors(k) = 1.0/square_errors(k);
  }

  double sum_imod2 = (square_errors.array() * model_intensities.array() * model_intensities.array()).sum();
  double sum_imod_iexp = (square_errors.array() * model_intensities.array() * exp_intensities.array()).sum();
  double c =  sum_imod_iexp / sum_imod2;

  if(std::fabs(offset) > 0.0000000001) {
    double sum_imod = (square_errors.array() * model_intensities.array()).sum();
    double constant =  sum_imod /sum_imod2;
    c += offset*constant;
  }
  return c;
}

double ChiScore::compute_offset(const Profile* exp_profile,
                                const Profile* model_profile) const {

  const Eigen::VectorXf& errors = exp_profile->get_errors();
  const Eigen::VectorXf& exp_intensities = exp_profile->get_intensities();
  const Eigen::VectorXf& model_intensities = model_profile->get_intensities();
  Eigen::VectorXf square_errors = errors.cwiseProduct(errors);
  for (int k = 0; k < square_errors.size(); k++) {
    square_errors(k) = 1.0/square_errors(k);
  }

  // compute constant
  double sum_imod = (square_errors.array() * model_intensities.array()).sum();
  double sum_imod2 = (square_errors.array() * (model_intensities.array() * model_intensities.array())).sum();
  double constant = sum_imod /sum_imod2;

  // compute scaling
  double sum_imod_iexp = (square_errors.array() * model_intensities.array() * exp_intensities.array()).sum();
  double c =  sum_imod_iexp / sum_imod2;

  // compute offset
  Eigen::VectorXf delta = exp_intensities - c * model_intensities;
  Eigen::VectorXf delta2 = constant * model_intensities;
  for (int k = 0; k < delta2.size(); k++) delta2(k) = 1.0 - delta2(k);

  double sum1 = (square_errors.array() * delta.array() * delta2.array()).sum();
  double sum2 = (square_errors.array() * delta2.array() * delta2.array()).sum();
  double offset = -sum1/sum2;

  return offset;
}

IMPSAXS_END_NAMESPACE

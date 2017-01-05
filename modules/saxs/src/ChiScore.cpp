/**
 *  \file ChiScore.h   \brief Basic SAXS scoring
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
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

  const IMP_Eigen::VectorXf& errors = exp_profile->get_errors();
  const IMP_Eigen::VectorXf& exp_intensities = exp_profile->get_intensities();
  const IMP_Eigen::VectorXf& model_intensities = model_profile->get_intensities();

  IMP_Eigen::VectorXf delta = exp_intensities - c * model_intensities;
  if(use_offset) delta.array() -= offset;

  for(unsigned int i=0; i<delta.size(); i++) {
    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta(i) / exp_intensities(i)) >= 1.0e-15)
      chi_square += square(delta(i)) / square(errors(i));
  }

  chi_square /= profile_size;
  return sqrt(chi_square);
}

double ChiScore::compute_scale_factor(const Profile* exp_profile,
                                      const Profile* model_profile,
                                      const double offset) const {
  double sum1 = 0.0, sum2 = 0.0;

  const IMP_Eigen::VectorXf& errors = exp_profile->get_errors();
  const IMP_Eigen::VectorXf& exp_intensities = exp_profile->get_intensities();
  const IMP_Eigen::VectorXf& model_intensities = model_profile->get_intensities();

  IMP_Eigen::VectorXf square_errors = errors.cwiseProduct(errors);
  for (unsigned int k = 0; k <square_errors.size(); k++) {
    square_errors(k) = 1.0/square_errors(k);
  }

  if(std::fabs(offset) > 0.0000000001) {
    IMP_Eigen::VectorXf exp_intensities_offset = exp_intensities.array() -offset;
    sum1 = ((model_intensities.cwiseProduct(exp_intensities_offset)).cwiseQuotient(square_errors)).sum();
  } else {
    sum1 = (square_errors.array() * model_intensities.array() * exp_intensities.array()).sum();
  }
  sum2 = (square_errors.array() * (model_intensities.array() * model_intensities.array())).sum();
  return sum1 / sum2;
}

double ChiScore::compute_offset(const Profile* exp_profile,
                                const Profile* model_profile) const {

  const IMP_Eigen::VectorXf& errors = exp_profile->get_errors();
  const IMP_Eigen::VectorXf& exp_intensities = exp_profile->get_intensities();
  const IMP_Eigen::VectorXf& model_intensities = model_profile->get_intensities();
  IMP_Eigen::VectorXf square_errors = errors.cwiseProduct(errors);

  double sum_iexp_imod = model_intensities.cwiseProduct(exp_intensities).cwiseQuotient(square_errors).sum();
  double sum_imod = model_intensities.cwiseQuotient(square_errors).sum();
  double sum_iexp = exp_intensities.cwiseQuotient(square_errors).sum();
  double sum_imod2 = model_intensities.cwiseProduct(model_intensities).cwiseQuotient(square_errors).sum();
  double sum_weight = square_errors.cwiseInverse().sum();

  double offset = sum_iexp_imod / sum_imod2 * sum_imod - sum_iexp;
  offset /= (sum_weight - sum_imod * sum_imod / sum_imod2);
  return offset;
}

IMPSAXS_END_NAMESPACE

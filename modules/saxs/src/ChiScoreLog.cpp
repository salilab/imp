/**
 * \file ChiScoreLog \brief scoring with log intensity
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/ChiScoreLog.h>

IMPSAXS_BEGIN_NAMESPACE

double ChiScoreLog::compute_scale_factor(const Profile* exp_profile,
                                         const Profile* model_profile,
                                         const double offset) const {
  IMP_UNUSED(offset);
  double sum1 = 0.0, sum2 = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  for (unsigned int k = 0; k < profile_size; k++) {
    double square_error =
        square(exp_profile->get_error(k) / exp_profile->get_intensity(k));
    double weight_tilda = model_profile->get_weight(k) / square_error;
    sum1 += weight_tilda * log(exp_profile->get_intensity(k) /
                               model_profile->get_intensity(k));
    sum2 += weight_tilda;
  }
  return std::exp(sum1 / sum2);
}

double ChiScoreLog::compute_score(const Profile* exp_profile,
                                  const Profile* model_profile,
                                  bool offset) const {
  IMP_UNUSED(offset);
  double c = compute_scale_factor(exp_profile, model_profile);

  double chi_square = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  // compute chi square
  for (unsigned int k = 0; k < profile_size; k++) {
    // in the theoretical profile the error equals to 1
    double square_error = square(log(exp_profile->get_error(k)));
    double weight_tilda = model_profile->get_weight(k) / square_error;
    double delta = log(exp_profile->get_intensity(k)) -
                   log(c * model_profile->get_intensity(k));

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta / (log(exp_profile->get_intensity(k)))) >= 1.0e-15)
      chi_square += weight_tilda * square(delta);
  }
  chi_square /= profile_size;
  return sqrt(chi_square);
}

double ChiScoreLog::compute_score(const Profile* exp_profile,
                                  const Profile* model_profile,
                                  double min_q, double max_q) const {
  double c = compute_scale_factor(exp_profile, model_profile);

  double chi_square = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  unsigned int interval_size = 0;
  // compute chi square
  for (unsigned int k = 0; k < profile_size; k++) {
    if (exp_profile->get_q(k) > max_q) break;
    if (exp_profile->get_q(k) >= min_q) {
      // in the theoretical profile the error equals to 1
      double square_error = square(log(exp_profile->get_error(k)));
      double weight_tilda = model_profile->get_weight(k) / square_error;
      double delta = log(exp_profile->get_intensity(k)) -
                     log(c * model_profile->get_intensity(k));

      // Exclude the uncertainty originated from limitation of floating number
      if (fabs(delta / (log(exp_profile->get_intensity(k)))) >= 1.0e-15) {
        chi_square += weight_tilda * square(delta);
        interval_size++;
      }
    }
  }
  if (interval_size > 0) chi_square /= interval_size;
  return sqrt(chi_square);
}

IMPSAXS_END_NAMESPACE

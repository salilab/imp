/**
 *  \file ChiScore.h   \brief Basic SAXS scoring
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/ChiScore.h>

IMPSAXS_BEGIN_NAMESPACE

Float ChiScore::compute_score(const Profile* exp_profile,
                              const Profile* model_profile,
                              bool use_offset) const {
  Float offset = 0.0;
  if (use_offset) offset = compute_offset(exp_profile, model_profile);
  Float c = compute_scale_factor(exp_profile, model_profile, offset);

  Float chi_square = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  // compute chi square
  for (unsigned int k = 0; k < profile_size; k++) {
    // in the theoretical profile the error equals to 1
    Float square_error = square(exp_profile->get_error(k));
    Float weight_tilda = model_profile->get_weight(k) / square_error;
    Float delta = exp_profile->get_intensity(k) - offset -
                  c * model_profile->get_intensity(k);

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta / exp_profile->get_intensity(k)) >= 1.0e-15)
      chi_square += weight_tilda * square(delta);
  }
  chi_square /= profile_size;
  return sqrt(chi_square);
}

Float ChiScore::compute_score(const Profile* exp_profile,
                              const Profile* model_profile, Float min_q,
                              Float max_q) const {
  Float offset = 0.0;
  Float c = compute_scale_factor(exp_profile, model_profile, offset);

  Float chi_square = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  unsigned int interval_size = 0;
  // compute chi square
  for (unsigned int k = 0; k < profile_size; k++) {
    if (exp_profile->get_q(k) > max_q) break;
    if (exp_profile->get_q(k) >= min_q) {
      // in the theoretical profile the error equals to 1
      Float square_error = square(exp_profile->get_error(k));
      Float weight_tilda = model_profile->get_weight(k) / square_error;
      Float delta = exp_profile->get_intensity(k) - offset -
                    c * model_profile->get_intensity(k);

      // Exclude the uncertainty originated from limitation of floating number
      if (fabs(delta / exp_profile->get_intensity(k)) >= 1.0e-15) {
        chi_square += weight_tilda * square(delta);
        interval_size++;
      }
    }
  }
  if (interval_size > 0) chi_square /= interval_size;
  return sqrt(chi_square);
}

Float ChiScore::compute_scale_factor(const Profile* exp_profile,
                                     const Profile* model_profile,
                                     const Float offset) const {
  Float sum1 = 0.0, sum2 = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  for (unsigned int k = 0; k < profile_size; k++) {
    Float square_error = square(exp_profile->get_error(k));
    Float weight_tilda = model_profile->get_weight(k) / square_error;

    sum1 += weight_tilda * model_profile->get_intensity(k) *
            (exp_profile->get_intensity(k) - offset);
    sum2 += weight_tilda * square(model_profile->get_intensity(k));
  }
  return sum1 / sum2;
}

Float ChiScore::compute_offset(const Profile* exp_profile,
                               const Profile* model_profile) const {
  Float sum_iexp_imod = 0.0, sum_imod = 0.0, sum_iexp = 0.0, sum_imod2 = 0.0;
  Float sum_weight = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  for (unsigned int k = 0; k < profile_size; k++) {
    Float square_error = square(exp_profile->get_error(k));
    Float weight_tilda = model_profile->get_weight(k) / square_error;

    sum_iexp_imod += weight_tilda * model_profile->get_intensity(k) *
                     exp_profile->get_intensity(k);
    sum_imod += weight_tilda * model_profile->get_intensity(k);
    sum_iexp += weight_tilda * exp_profile->get_intensity(k);
    sum_imod2 += weight_tilda * square(model_profile->get_intensity(k));
    sum_weight += weight_tilda;
  }
  Float offset = sum_iexp_imod / sum_imod2 * sum_imod - sum_iexp;
  offset /= (sum_weight - sum_imod * sum_imod / sum_imod2);
  return offset;
}

IMPSAXS_END_NAMESPACE

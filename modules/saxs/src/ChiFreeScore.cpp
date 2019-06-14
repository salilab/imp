/**
 *  \file ChiFreeScore.h   \brief Chi free SAXS score
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/ChiFreeScore.h>
#include <boost/random/uniform_real.hpp>
#include <algorithm>

IMPSAXS_BEGIN_NAMESPACE

namespace {
bool comp_function(std::pair<double, double> a, std::pair<double, double> b) {
  return (a.first < b.first);
}
}  // anonymous namespace

// TODO: currently assumes uniform sampling of experimental profile
double ChiFreeScore::compute_score(const Profile* exp_profile,
                                   const Profile* model_profile,
                                   bool use_offset) const {
  if (model_profile->size() != exp_profile->size()) {
    IMP_THROW("ChiFreeScore::compute_score is supported "
                  << "only for profiles with the same q values!",
              ValueException);
  }

  const_cast<ChiFreeScore*>(this)->last_scale_updated_ = false;
  // init random number generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type rng;

  boost::uniform_real<> uni_dist(0, 1);
  boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni(
      rng, uni_dist);

  Vector<std::pair<double, double> > chis(K_);
  unsigned int bin_size = std::floor((double)exp_profile->size()) / ns_;


  IMP_NEW(Profile, exp_profile_selection, ());
  IMP_NEW(Profile, model_profile_selection, ());
  Eigen::VectorXf qs(ns_), errors(ns_);
  Eigen::VectorXf exp_intensities(ns_), model_intensities(ns_);

  // repeat K_ times
  for (unsigned int k = 0; k < K_; k++) {
    // select a point in each interval
    for (unsigned int i = 0; i < ns_; i++) {
      double prob = uni();
      unsigned int profile_index =
          algebra::get_rounded((double)i * bin_size + prob * bin_size);
      if (profile_index < exp_profile->size()) {
        qs(i) = exp_profile->get_q(profile_index);
        errors(i) = exp_profile->get_error(profile_index);
        exp_intensities(i) = exp_profile->get_intensity(profile_index);
        model_intensities(i) = model_profile->get_intensity(profile_index);
      }
    }
    exp_profile_selection->set_qs(qs);
    exp_profile_selection->set_intensities(exp_intensities);
    exp_profile_selection->set_errors(errors);
    model_profile_selection->set_qs(qs);
    model_profile_selection->set_intensities(model_intensities);

    // compute chi
    double offset = 0.0;
    if (use_offset)
      offset = compute_offset(exp_profile_selection, model_profile_selection);
    double c = compute_scale_factor(exp_profile_selection,
                                   model_profile_selection, offset);
    double chi_square = 0.0;
    unsigned int profile_size = std::min(model_profile_selection->size(),
                                         exp_profile_selection->size());
    // compute chi square
    for (unsigned int i = 0; i < profile_size; i++) {
      // in the theoretical profile the error equals to 1
      double square_error = square(exp_profile_selection->get_error(i));
      double weight_tilda =
          model_profile_selection->get_weight(i) / square_error;
      double delta = exp_profile_selection->get_intensity(i) - offset -
                    c * model_profile_selection->get_intensity(i);
      // Exclude the uncertainty originated from limitation of floating number
      if (fabs(delta / exp_profile_selection->get_intensity(i)) >= 1.0e-15)
        chi_square += weight_tilda * square(delta);
    }
    chi_square /= profile_size;
    chis[k] = std::make_pair(chi_square, c);
  }
  unsigned int n = K_ / 2;
  std::nth_element(chis.begin(), chis.begin() + n, chis.end(), comp_function);
  const_cast<ChiFreeScore*>(this)->last_scale_updated_ = true;
  const_cast<ChiFreeScore*>(this)->last_scale_ = chis[n].second;
  return sqrt(chis[n].first);
}

double ChiFreeScore::compute_scale_factor(const Profile* exp_profile,
                                          const Profile* model_profile,
                                          const double offset) const {
  if (last_scale_updated_) return last_scale_;
  double sum1 = 0.0, sum2 = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  for (unsigned int k = 0; k < profile_size; k++) {
    double square_error = square(exp_profile->get_error(k));
    double weight_tilda = model_profile->get_weight(k) / square_error;

    sum1 += weight_tilda * model_profile->get_intensity(k) *
            (exp_profile->get_intensity(k) - offset);
    sum2 += weight_tilda * square(model_profile->get_intensity(k));
  }
  return sum1 / sum2;
}

double ChiFreeScore::compute_offset(const Profile* exp_profile,
                                    const Profile* model_profile) const {
  double sum_iexp_imod = 0.0, sum_imod = 0.0, sum_iexp = 0.0, sum_imod2 = 0.0;
  double sum_weight = 0.0;
  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile->size());
  for (unsigned int k = 0; k < profile_size; k++) {
    double square_error = square(exp_profile->get_error(k));
    double weight_tilda = model_profile->get_weight(k) / square_error;

    sum_iexp_imod += weight_tilda * model_profile->get_intensity(k) *
                     exp_profile->get_intensity(k);
    sum_imod += weight_tilda * model_profile->get_intensity(k);
    sum_iexp += weight_tilda * exp_profile->get_intensity(k);
    sum_imod2 += weight_tilda * square(model_profile->get_intensity(k));
    sum_weight += weight_tilda;
  }
  double offset = sum_iexp_imod / sum_imod2 * sum_imod - sum_iexp;
  offset /= (sum_weight - sum_imod * sum_imod / sum_imod2);
  return offset;
}

IMPSAXS_END_NAMESPACE

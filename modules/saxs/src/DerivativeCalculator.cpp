/**
 * \file DerivativeCalculator \brief A class for computing SAXS derivatives
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/DerivativeCalculator.h>
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/Score.h>
#include <IMP/saxs/utility.h>

#include <boost/math/special_functions/sinc.hpp>

IMPSAXS_BEGIN_NAMESPACE

DerivativeCalculator::DerivativeCalculator(const Profile& exp_profile) :
  exp_profile_(exp_profile) {}

void DerivativeCalculator::compute_sinc_cos(Float pr_resolution,
                                            Float max_distance,
                                            const Profile& model_profile,
                                std::vector<Floats>& output_values) const
{
  //can be input
  unsigned int nr=algebra::get_rounded(max_distance/pr_resolution) + 1;
  output_values.clear();
  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  Floats r_size(nr, 0.0);
  output_values.insert(output_values.begin(),
                       profile_size, r_size);
  for(unsigned int iq = 0; iq<profile_size; iq++) {
    Float q = model_profile.get_q(iq);
    for (unsigned int ir=0; ir<nr; ir++) {
      Float r = pr_resolution * ir;
      Float qr = q * r;
      if(fabs(qr) < 1.0e-16) {
        output_values[iq][ir] = 0;
      } else {
        output_values[iq][ir] = (boost::math::sinc_pi(qr) - cos(qr)) /square(r);
      }
    }
  }
}

void DerivativeCalculator::compute_profile_difference(
                                       const Profile& model_profile,
                                       const Float c, const Float offset,
                                       Floats& profile_diff) const
{
  // compute difference of intensities and squares of weight
  // profile_diff[q] = e_q * weight_tilda * (I_exp[q] - c*I_mod[q] + offset)
  // e_q = exp( -0.23 * q*q )
  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  profile_diff.clear();
  profile_diff.resize(profile_size, 0.0);

  for (unsigned int iq=0; iq<profile_size; iq++) {
    Float delta = exp_profile_.get_intensity(iq)
                  - c * model_profile.get_intensity(iq) + offset;
    Float square_error = square(exp_profile_.get_error(iq));
    Float weight_tilda = model_profile.get_weight(iq) / square_error;

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_profile_.get_intensity(iq)) < IMP_SAXS_DELTA_LIMIT)
      delta = 0.0;
    // Float E_q = std::exp( - exp_profile_. modulation_function_parameter_
    //                    * square( exp_profile_.get_q(iq)));
    //profile_diff[iq] = E_q * weight_tilda * delta;
    profile_diff[iq] = weight_tilda * delta;
  }
}

/*
compute derivative for each particle in particles1 with respect to particles2
SCORING function : chi
For calculation in real space the quantity Delta(r) is needed to get
derivatives of an atom
Delta(r) = f_iatom * sum_i f_i delta(r-r_{i,iatom}) (x_iatom-x_i)
*/
void DerivativeCalculator::compute_chi_derivative(const Profile& model_profile,
                                   const Particles& particles1,
                                   const Particles& particles2,
                         std::vector<algebra::Vector3D>& derivatives,
                                   bool use_offset) const {

  Profile resampled_profile(exp_profile_.get_min_q(),
                            exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  Score score(exp_profile_);
  score.resample(model_profile, resampled_profile);

  algebra::Vector3D delta_q, chi_derivative;

  // Pre-compute common parameters for faster calculation
  Floats profile_diff;
  Float offset = 0.0;
  if(use_offset) offset = score.compute_offset(model_profile);
  Float c = score.compute_scale_factor(model_profile);
  compute_profile_difference(model_profile, c, offset, profile_diff);


  // estimate upper limit on max_distance
  Float max_distance = compute_max_distance(particles1, particles2);
  DeltaDistributionFunction delta_dist(particles2, max_distance);
  std::vector<Floats> sinc_cos_values; // (sinc(qr) - cos(qr)) / (r*r)
  compute_sinc_cos(delta_dist.get_bin_size(), max_distance,
                   model_profile, sinc_cos_values);
  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  derivatives.clear();
  derivatives.resize(particles1.size());
  for (unsigned int iatom=0; iatom<particles1.size(); iatom++) {
    // Compute a delta distribution per an atom
    delta_dist.calculate_derivative_distribution(particles1[iatom]);
    chi_derivative = algebra::Vector3D(0.0, 0.0, 0.0);

    for (unsigned int iq=0; iq<profile_size; iq++) {
      delta_q = algebra::Vector3D(0.0, 0.0, 0.0);

      for (unsigned int ir=0; ir<delta_dist.size(); ir++) {
        // delta_dist.distribution = sum_i [f_k(0) * f_i(0) * (x_k - x_i)]
        // sinc_cos_values = (sinc(qr) - cos_(qr)) / (r*r)
        delta_q += delta_dist[ir] * sinc_cos_values[iq][ir];
      }
      // profile_diff = weight_tilda * (I_exp - c*I_model)
      // e_q = exp( -0.23 * q*q )
      Float E_q = std::exp( - exp_profile_. modulation_function_parameter_
                       * square( exp_profile_.get_q(iq)));
      chi_derivative += E_q * delta_q * profile_diff[iq];
    }
    derivatives[iatom] = 4 * c * chi_derivative;
  }
}

IMPSAXS_END_NAMESPACE

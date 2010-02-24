/**
 *  \file Score.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/Score.h>
#include <IMP/algebra/utility.h>
#include <IMP/saxs/utility.h>

#include <map>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

Score::Score(const Profile& exp_profile) : exp_profile_(exp_profile)
{}

void Score::resample(const Profile& model_profile,
                     Profile& resampled_profile) const
{
  // map of q values for fast search
  std::map<float, unsigned int> q_mapping;
  for (unsigned int k=0; k<model_profile.size(); k++) {
    q_mapping[model_profile.get_q(k)] = k;
  }

  for (unsigned int k=0; k<exp_profile_.size(); k++) {
    Float q = exp_profile_.get_q(k);
    std::map<float, unsigned int>::iterator it = q_mapping.lower_bound(q);
    if(it == q_mapping.end()) break;
    unsigned int i = it->second;
    if(i == 0) {
      resampled_profile.add_entry(q, model_profile.get_intensity(i));
    } else {
      Float delta_q = model_profile.get_q(i)-model_profile.get_q(i-1);
      if(delta_q <= 1.0e-16) {
        resampled_profile.add_entry(q, model_profile.get_intensity(i));
      } else {
        Float alpha = (q - model_profile.get_q(i-1)) / delta_q;
        if(alpha > 1.0) alpha = 1.0; // handle rounding errors
        Float intensity = model_profile.get_intensity(i-1)
          + (alpha)*(model_profile.get_intensity(i)
                     - model_profile.get_intensity(i-1));
        resampled_profile.add_entry(q, intensity);
      }
    }
  }
}

Float Score::compute_scale_factor(const Profile& model_profile,
                                  const Float offset) const
{
  Float sum1=0.0, sum2=0.0;
  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  for (unsigned int k=0; k<profile_size; k++) {
    Float square_error = square(exp_profile_.get_error(k));
    Float weight_tilda = model_profile.get_weight(k) / square_error;

    sum1 += weight_tilda * model_profile.get_intensity(k)
                         * (exp_profile_.get_intensity(k) + offset);
    sum2 += weight_tilda * square(model_profile.get_intensity(k));
  }
  // std::cerr << "c = " << sum1 / sum2 << std::endl;
  return sum1 / sum2;
}

Float Score::compute_offset(const Profile& model_profile) const {
  Float sum_iexp_imod=0.0, sum_imod=0.0, sum_iexp=0.0, sum_imod2=0.0;
  Float sum_weight=0.0;
  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  for (unsigned int k=0; k<profile_size; k++) {
    Float square_error = square(exp_profile_.get_error(k));
    Float weight_tilda = model_profile.get_weight(k) / square_error;

    sum_iexp_imod += weight_tilda * model_profile.get_intensity(k)
                                  * exp_profile_.get_intensity(k);
    sum_imod += weight_tilda * model_profile.get_intensity(k);
    sum_iexp += weight_tilda * exp_profile_.get_intensity(k);
    sum_imod2 += weight_tilda * square(model_profile.get_intensity(k));
    sum_weight += weight_tilda;
  }
  Float offset = sum_iexp_imod / sum_imod2 * sum_imod - sum_iexp;
  offset /= (sum_weight - sum_imod*sum_imod/sum_imod2);
  return offset;
}

Float Score::compute_chi_square_score(const Profile& model_profile,
                                      bool use_offset,
                                      const std::string fit_file_name) const
{
  Profile resampled_profile(exp_profile_.get_min_q(),
                            exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  resample(model_profile, resampled_profile);
  return compute_chi_square_score_internal(
                            resampled_profile, fit_file_name, use_offset);
}

Float Score::compute_chi_square_score_internal(
                             const Profile& model_profile,
                             const std::string& fit_file_name,
                             bool use_offset) const
{
  Float offset = 0.0;
  if(use_offset) offset = compute_offset(model_profile);
  Float c = compute_scale_factor(model_profile, offset);
  Float chi_square =
    compute_chi_square_score_internal(model_profile, c, offset);

  if(fit_file_name.length() > 0) {
    write_SAXS_fit_file(fit_file_name, model_profile,
                        chi_square, c, offset);
  }
  return chi_square;
}

/*
compute SAXS Chi square of experimental data and model
weight_tilda function w_tilda(q) = w(q) / sigma_exp^2
*/
// TODO: define a weight function (w(q) = 1, q^2, or hybrid)
Float Score::compute_chi_square_score_internal(
                             const Profile& model_profile,
                             const Float c, const Float offset) const
{
  Float chi_square = 0.0;
  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  // compute chi square
  for (unsigned int k=0; k<profile_size; k++) {
    // in the theoretical profile the error equals to 1
    Float square_error = square(exp_profile_.get_error(k));
    Float weight_tilda = model_profile.get_weight(k) / square_error;
    Float delta = exp_profile_.get_intensity(k) + offset
                    - c * model_profile.get_intensity(k);

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_profile_.get_intensity(k)) >= IMP_SAXS_DELTA_LIMIT)
      chi_square += weight_tilda * square(delta);
  }
  chi_square /= profile_size;
  return chi_square;
}

void Score::compute_sinc_cos(Float pr_resolution, Float max_distance,
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
        output_values[iq][ir] = (sinc(qr) - cos(qr)) / square(r);
      }
    }
  }
}

void Score::compute_profile_difference(const Profile& model_profile,
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
    Float E_q = std::exp( - exp_profile_. modulation_function_parameter_
                       * square( exp_profile_.get_q(iq)));
    profile_diff[iq] = E_q * weight_tilda * delta;
  }
}

void Score::compute_chi_derivative(const Profile& model_profile,
                                   const Particles& particles1,
                                   const Particles& particles2,
                         std::vector<algebra::VectorD<3> >& derivatives,
                                   bool use_offset) const {

  Profile resampled_profile(exp_profile_.get_min_q(),
                            exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  resample(model_profile, resampled_profile);
  compute_chi_real_derivative(resampled_profile, particles1, particles2,
                              derivatives, use_offset);
}

/*
compute derivative for each particle in particles1 with respect to particles2
SCORING function : chi
For calculation in real space the quantity Delta(r) is needed to get
derivatives of an atom
Delta(r) = f_iatom * sum_i f_i delta(r-r_{i,iatom}) (x_iatom-x_i)
*/
void Score::compute_chi_real_derivative(const Profile& model_profile,
                                 const Particles& particles1,
                                 const Particles& particles2,
                                 std::vector<algebra::VectorD<3> >& derivatives,
                                 bool use_offset) const
{
  algebra::VectorD<3> delta_q, chi_derivative;

  // Pre-compute common parameters for faster calculation
  Floats profile_diff;
  Float offset = 0.0;
  if(use_offset) offset = compute_offset(model_profile);
  Float c = compute_scale_factor(model_profile);
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
    chi_derivative = algebra::VectorD<3>(0.0, 0.0, 0.0);

    for (unsigned int iq=0; iq<profile_size; iq++) {
      delta_q = algebra::VectorD<3>(0.0, 0.0, 0.0);

      for (unsigned int ir=0; ir<delta_dist.size(); ir++) {
        // delta_dist.distribution = sum_i [f_k(0) * f_i(0) * (x_k - x_i)]
        // sinc_cos_values = (sinc(qr) - cos_(qr)) / (r*r)
        delta_q += delta_dist[ir] * sinc_cos_values[iq][ir];
      }
      // profile_diff = weight_tilda * (I_exp - c*I_model)
      // e_q = exp( -0.23 * q*q )
      chi_derivative += delta_q * profile_diff[iq];
    }
    derivatives[iatom] = 4 * c * chi_derivative;
  }
}

void Score::write_SAXS_fit_file(const std::string& file_name,
                                const Profile& model_profile,
                                const Float chi_square,
                                const Float c, const Float offset) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }

  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  // header line
  out_file.precision(15);
  out_file << "# SAXS profile: number of points = " << profile_size
           << ", q_min = " << exp_profile_.get_min_q()
           << ", q_max = " << exp_profile_.get_max_q();
  out_file << ", delta_q = " << exp_profile_.get_delta_q() << std::endl;

  out_file.setf(std::ios::showpoint);
  out_file << "# offset = " << offset << ", scaling c = " << c
           << ", Chi = " << sqrt(chi_square) << std::endl;
  out_file << "#  q       exp_intensity   model_intensity"
           << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < profile_size; i++) {
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(5);
    out_file << exp_profile_.get_q(i) << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << exp_profile_.get_intensity(i)  << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << model_profile.get_intensity(i)*c - offset << std::endl;
  }
  out_file.close();
}

IMPSAXS_END_NAMESPACE

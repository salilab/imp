/**
 *  \file SAXSScore.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/SAXSScore.h>
#include <IMP/algebra/utility.h>
#include <IMP/saxs/utility.h>

#include <map>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

SAXSScore::SAXSScore(FormFactorTable* ff_table,
                     SAXSProfile* exp_saxs_profile) :
ff_table_(ff_table), exp_saxs_profile_(exp_saxs_profile)
{}

void SAXSScore::resample(const SAXSProfile& model_saxs_profile,
                         SAXSProfile& resampled_profile) const {
  // map of q values for fast search
  std::map<float, unsigned int> q_mapping;
  for (unsigned int k=0; k<model_saxs_profile.size(); k++) {
    q_mapping[model_saxs_profile.get_q(k)] = k;
  }

  for (unsigned int k=0; k<exp_saxs_profile_->size(); k++) {
    Float q = exp_saxs_profile_->get_q(k);
    std::map<float, unsigned int>::iterator it = q_mapping.lower_bound(q);
    if(it == q_mapping.end()) break;
    unsigned int i = it->second;
    if(i == 0) {
      resampled_profile.add_entry(q, model_saxs_profile.get_intensity(i));
    } else {
      Float delta_q = model_saxs_profile.get_q(i)-model_saxs_profile.get_q(i-1);
      if(delta_q <= 1.0e-16) {
        resampled_profile.add_entry(q, model_saxs_profile.get_intensity(i));
      } else {
        Float alpha = (q - model_saxs_profile.get_q(i-1)) / delta_q;
        Float intensity = algebra::simple_iterpolate(alpha,
                                   model_saxs_profile.get_intensity(i-1),
                                   model_saxs_profile.get_intensity(i));
        resampled_profile.add_entry(q, intensity);
      }
    }
  }
}

Float SAXSScore::compute_scale_factor(const SAXSProfile& model_saxs_profile,
                                      const Float offset) const
{
  Float sum1=0.0, sum2=0.0;
  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  for (unsigned int k=0; k<profile_size; k++) {
    Float square_error = square(exp_saxs_profile_->get_error(k));
    Float weight_tilda = model_saxs_profile.get_weight(k) / square_error;

    sum1 += weight_tilda * model_saxs_profile.get_intensity(k)
                         * (exp_saxs_profile_->get_intensity(k) + offset);
    sum2 += weight_tilda * square(model_saxs_profile.get_intensity(k));
  }
  // std::cerr << "c = " << sum1 / sum2 << std::endl;
  return sum1 / sum2;
}

Float SAXSScore::compute_offset(const SAXSProfile& model_saxs_profile) const {
  Float sum_iexp_imod=0.0, sum_imod=0.0, sum_iexp=0.0, sum_imod2=0.0;
  Float sum_weight=0.0;
  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  for (unsigned int k=0; k<profile_size; k++) {
    Float square_error = square(exp_saxs_profile_->get_error(k));
    Float weight_tilda = model_saxs_profile.get_weight(k) / square_error;

    sum_iexp_imod += weight_tilda * model_saxs_profile.get_intensity(k)
                                  * exp_saxs_profile_->get_intensity(k);
    sum_imod += weight_tilda * model_saxs_profile.get_intensity(k);
    sum_iexp += weight_tilda * exp_saxs_profile_->get_intensity(k);
    sum_imod2 += weight_tilda * square(model_saxs_profile.get_intensity(k));
    sum_weight += weight_tilda;
  }
  Float offset = sum_iexp_imod / sum_imod2 * sum_imod - sum_iexp;
  offset /= (sum_weight - sum_imod*sum_imod/sum_imod2);
  return offset;
}

Float SAXSScore::compute_chi_square_score(
                             const SAXSProfile& model_saxs_profile,
                             const std::string fit_file_name,
                             bool use_offset) const
{
  SAXSProfile resampled_profile(exp_saxs_profile_->get_min_q(),
                                exp_saxs_profile_->get_max_q(),
                                exp_saxs_profile_->get_delta_q(),
                                ff_table_);
  resample(model_saxs_profile, resampled_profile);
  return compute_chi_square_score_internal(
                            resampled_profile, fit_file_name, use_offset);
}

Float SAXSScore::compute_chi_square_score_internal(
                             const SAXSProfile& model_saxs_profile,
                             const std::string& fit_file_name,
                             bool use_offset) const
{
  Float offset = 0.0;
  if(use_offset) offset = compute_offset(model_saxs_profile);
  std::cerr << "offset = " << offset << std::endl;
  Float c = compute_scale_factor(model_saxs_profile, offset);
  Float chi_square = compute_chi_square_score_internal(
                                          model_saxs_profile, c, offset);

  if(fit_file_name.length() > 0) {
    write_SAXS_fit_file(fit_file_name, model_saxs_profile,
                        chi_square, c, offset);
  }
  return chi_square;
}

/*
compute SAXS Chi square of experimental data and model
weight_tilda function w_tilda(q) = w(q) / sigma_exp^2
*/
// TODO: define a weight function (w(q) = 1, q^2, or hybrid)
Float SAXSScore::compute_chi_square_score_internal(
                             const SAXSProfile& model_saxs_profile,
                             const Float c, const Float offset) const
{
  Float chi_square = 0.0;
  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  // compute chi square
  for (unsigned int k=0; k<profile_size; k++) {
    // in the theoretical profile the error equals to 1
    Float square_error = square(exp_saxs_profile_->get_error(k));
    Float weight_tilda = model_saxs_profile.get_weight(k) / square_error;
    Float delta = exp_saxs_profile_->get_intensity(k) + offset
                    - c * model_saxs_profile.get_intensity(k);

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_saxs_profile_->get_intensity(k)) >= IMP_SAXS_DELTA_LIMIT)
      chi_square += weight_tilda * square(delta);
  }
  chi_square /= profile_size;
  return chi_square;
}

void SAXSScore::compute_sinc_cos(Float pr_resolution, Float max_distance,
                                 const SAXSProfile& model_saxs_profile,
                                 std::vector<Floats>& output_values) const
{
  unsigned int nr=algebra::round(max_distance/pr_resolution) + 1; //can be input
  output_values.clear();
  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  Floats r_size(nr, 0.0);
  output_values.insert(output_values.begin(),
                       profile_size, r_size);
  for(unsigned int iq = 0; iq<profile_size; iq++) {
    Float q = model_saxs_profile.get_q(iq);
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

void SAXSScore::compute_profile_difference(
                       const SAXSProfile& model_saxs_profile,
                       const Float c, const Float offset,
                       Floats& profile_diff) const
{
  // compute difference of intensities and squares of weight
  // profile_diff[q] = e_q * weight_tilda * (I_exp[q] - c*I_mod[q] + offset)
  // e_q = exp( -0.23 * q*q )
  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  profile_diff.clear();
  profile_diff.resize(profile_size, 0.0);

  for (unsigned int iq=0; iq<profile_size; iq++) {
    Float delta = exp_saxs_profile_->get_intensity(iq)
                  - c * model_saxs_profile.get_intensity(iq) + offset;
    Float square_error = square(exp_saxs_profile_->get_error(iq));
    Float weight_tilda = model_saxs_profile.get_weight(iq) / square_error;

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_saxs_profile_->get_intensity(iq)) < IMP_SAXS_DELTA_LIMIT)
      delta = 0.0;
    Float E_q = std::exp( - exp_saxs_profile_-> modulation_function_parameter_
                       * square( exp_saxs_profile_->get_q(iq) ) );
    profile_diff[iq] = E_q * weight_tilda * delta;
  }
}

Float SAXSScore::compute_chi_derivative(const SAXSProfile& model_saxs_profile,
                                  const std::vector<Particle*>& particles,
                                  std::vector<algebra::Vector3D>& derivatives,
                                  bool use_offset) const {

  SAXSProfile resampled_profile(exp_saxs_profile_->get_min_q(),
                                exp_saxs_profile_->get_max_q(),
                                exp_saxs_profile_->get_delta_q(),
                                ff_table_);
  resample(model_saxs_profile, resampled_profile);
  return compute_chi_real_derivative(resampled_profile, particles,
                                     derivatives, use_offset);
}

/*
compute derivative for each particle
SCORING function : chi
For calculation in real space the quantity Delta(r) is needed to get
derivatives of an atom
Delta(r) = f_iatom * sum_i f_i delta(r-r_{i,iatom}) (x_iatom-x_i)
*/
Float SAXSScore::compute_chi_real_derivative(
                           const SAXSProfile& model_saxs_profile,
                           const std::vector<Particle*>& particles,
                           std::vector<algebra::Vector3D>& derivatives,
                           bool use_offset) const
{
  algebra::Vector3D delta_q, chi_derivative;

  // Pre-compute common parameters for faster calculation
  Floats profile_diff;
  Float offset = 0.0;
  if(use_offset) offset = compute_offset(model_saxs_profile);
  Float c = compute_scale_factor(model_saxs_profile);
  Float chi_square =
    compute_chi_square_score_internal(model_saxs_profile, c, offset);
  compute_profile_difference(model_saxs_profile, c, offset, profile_diff);

  DeltaDistributionFunction delta_dist(ff_table_, particles);

  std::vector<Floats> sinc_cos_values; // (sinc(qr) - cos(qr)) / (r*r)
  compute_sinc_cos(delta_dist.get_bin_size(), delta_dist.get_max_distance(),
                   model_saxs_profile, sinc_cos_values);
  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  derivatives.clear();
  derivatives.resize(particles.size());
  for (unsigned int iatom=0; iatom<particles.size(); iatom++) {
    // Compute a delta distribution per an atom
    delta_dist.calculate_derivative_distribution(particles[iatom]);
    chi_derivative = algebra::Vector3D(0.0, 0.0, 0.0);

    for (unsigned int iq=0; iq<profile_size; iq++) {
      delta_q = algebra::Vector3D(0.0, 0.0, 0.0);

      for (unsigned int ir=0; ir<delta_dist.size(); ir++) {
        // delta_dist.distribution = sum_i [f_k(0) * f_i(0) * (x_k - x_i)]
        // sinc_cos_values = (sinc(qr) - cos_(qr)) / (r*r)
        delta_q += delta_dist.distribution_[ir] * sinc_cos_values[iq][ir];
      }
      // profile_diff = weight_tilda * (I_exp - c*I_model)
      // e_q = exp( -0.23 * q*q )
      chi_derivative += delta_q * profile_diff[iq];
    }
    derivatives[iatom] = 4 * c * chi_derivative;
  }
  return chi_square;
}

void SAXSScore::write_SAXS_fit_file(const std::string& file_name,
                                    const SAXSProfile& model_saxs_profile,
                                    const Float chi_square,
                                    const Float c, const Float offset) const {
  std::ofstream out_file(file_name.c_str());

  if (!out_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }

  unsigned int profile_size = std::min(model_saxs_profile.size(),
                                       exp_saxs_profile_->size());
  // header line
  out_file.precision(15);
  out_file << "# SAXS profile: number of points = " << profile_size
           << ", q_min = " << exp_saxs_profile_->get_min_q()
           << ", q_max = " << exp_saxs_profile_->get_max_q();
  out_file << ", delta_q = " << exp_saxs_profile_->get_delta_q() << std::endl;

  out_file.setf(std::ios::showpoint);
  out_file << "# offset = " << offset << ", scaling c = " << c
           << ", Chi = " << sqrt(chi_square) << std::endl;
  out_file << "#       q             exp_intensity    model_intensity"
           << std::endl;

  // Main data
  for (unsigned int i = 0; i < profile_size; i++) {
    out_file.setf(std::ios::left);
    out_file.width(20);
    out_file.fill('0');
    out_file << exp_saxs_profile_->get_q(i) << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << exp_saxs_profile_->get_intensity(i)  << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << model_saxs_profile.get_intensity(i)*c - offset << std::endl;
  }
  out_file.close();
}

IMPSAXS_END_NAMESPACE

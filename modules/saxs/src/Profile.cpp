/**
 *  \file Profile.cpp   \brief A class for profile storing and computation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/utility.h>
#include <IMP/saxs/internal/sinc_function.h>
#include <IMP/saxs/internal/exp_function.h>

#include <IMP/base/math.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ParabolicFit.h>
#include <IMP/algebra/LinearFit.h>
#include <IMP/constants.h>
#include <IMP/base/random.h>

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/math/special_functions/sinc.hpp>
#include <boost/random/normal_distribution.hpp>

#include <fstream>
#include <string>

#define IMP_SAXS_DELTA_LIMIT 1.0e-15

IMPSAXS_BEGIN_NAMESPACE

const Float Profile::modulation_function_parameter_ = 0.23;

Profile::Profile(Float qmin, Float qmax, Float delta)
    : base::Object("profile%1%"),
      min_q_(qmin),
      max_q_(qmax),
      delta_q_(delta),
      c1_(10),
      c2_(10),
      experimental_(false),
      average_radius_(1.58),
      average_volume_(17.5),
      id_(0) {
  set_was_used(true);
  ff_table_ = default_form_factor_table();
}

Profile::Profile(const String& file_name, bool fit_file)
    : base::Object("profile%1%"),
      experimental_(true),
      name_(file_name),
      id_(0) {
  set_was_used(true);
  if (fit_file) experimental_ = false;
  read_SAXS_file(file_name, fit_file);
}

void Profile::init() {
  q_.clear();
  intensity_.clear();
  error_.clear();
  int number_of_q_entries = (int)std::ceil((max_q_ - min_q_) / delta_q_) + 1;
  q_.resize(number_of_q_entries);
  intensity_.resize(number_of_q_entries);
  error_.resize(number_of_q_entries);

  for (int i = 0; i < number_of_q_entries; i++) {
    q_[i] = min_q_ + i * delta_q_;
  }
}

void Profile::read_SAXS_file(const String& file_name, bool fit_file) {
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  bool with_error = false;
  std::string line;
  double q, intensity, error;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line);  // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() < 2 || split_results.size() > 5) continue;
    q = atof(split_results[0].c_str());
    if (fit_file) {
      if (split_results.size() != 3) continue;
      intensity = atof(split_results[2].c_str());
    } else {
      intensity = atof(split_results[1].c_str());
    }

    // validity checks
    if (fabs(intensity) < IMP_SAXS_DELTA_LIMIT)
      continue;             // skip zero intensities
    if (intensity < 0.0) {  // negative intensity
      IMP_WARN("Negative intensity value: "
               << line << " skipping remaining profile points" << std::endl);
      break;
    }
    error = 1.0;
    if (split_results.size() >= 3) {
      error = atof(split_results[2].c_str());
      if (fabs(error) < IMP_SAXS_DELTA_LIMIT) {
        error = 0.05 * intensity;
        if (fabs(error) < IMP_SAXS_DELTA_LIMIT) continue;  // skip entry
      }
      with_error = true;
    }
    add_entry(q, intensity, error);
  }
  in_file.close();

  // determine qmin, qmax and delta
  if (size() > 1) {
    min_q_ = q_[0];
    max_q_ = q_[size() - 1];

    if (is_uniform_sampling()) {
      // To minimize rounding errors, by averaging differences of q
      Float diff = 0.0;
      for (unsigned int i = 1; i < size(); i++) diff += q_[i] - q_[i - 1];
      delta_q_ = diff / (size() - 1);
    } else {
      delta_q_ = (max_q_ - min_q_) / (size() - 1);
    }
  }

  IMP_LOG_TERSE("read_SAXS_file: " << file_name << " size= " << size()
                                   << " delta= " << delta_q_
                                   << " min_q= " << min_q_
                                   << " max_q= " << max_q_ << std::endl);

  // saxs_read: No experimental error specified, add errors
  if (!with_error) {
    add_errors();
    IMP_LOG_TERSE("read_SAXS_file: No experimental error specified"
                  << " -> error added " << std::endl);
  }
}

void Profile::add_errors() {
  // init random number generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type rng;
  // rng.seed(static_cast<unsigned int>(std::time(0)));

  // init distribution
  typedef boost::poisson_distribution<> poisson;
  poisson poisson_dist(10.0);
  typedef boost::variate_generator<base_generator_type&, poisson>
      poisson_generator_type;
  poisson_generator_type poisson_rng(rng, poisson_dist);

  for (unsigned int i = 0; i < size(); i++) {
    double ra = std::abs(poisson_rng() / 10.0 - 1.0) + 1.0;

    // 3% of error, scaled by 5q + poisson distribution
    error_[i] = 0.03 * intensity_[i] * 5.0 * (q_[i] + 0.001) * ra;
  }
  experimental_ = true;
}

void Profile::add_noise(Float percentage) {
  // init random number generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type rng;

  // init distribution
  typedef boost::poisson_distribution<> poisson;
  poisson poisson_dist(10.0);
  typedef boost::variate_generator<base_generator_type&, poisson>
      poisson_generator_type;
  poisson_generator_type poisson_rng(rng, poisson_dist);

  for (unsigned int i = 0; i < size(); i++) {
    double random_number = poisson_rng() / 10.0 - 1.0;
    // X% of intensity weighted by (1+q) + poisson distribution
    intensity_[i] += percentage * intensity_[i] * (q_[i] + 1.0) * random_number;
  }
}

bool Profile::is_uniform_sampling() const {
  if (size() <= 1) return false;

  Float curr_diff = q_[1] - q_[0];
  Float epsilon = curr_diff / 10;
  for (unsigned int i = 2; i < size(); i++) {
    Float diff = q_[i] - q_[i - 1];
    if (fabs(curr_diff - diff) > epsilon) return false;
  }
  return true;
}

void Profile::write_SAXS_file(const String& file_name, Float max_q) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  // header line
  out_file << "# SAXS profile: number of points = " << size()
           << ", q_min = " << min_q_ << ", q_max = ";
  if (max_q > 0)
    out_file << max_q;
  else
    out_file << max_q_;
  out_file << ", delta_q = " << delta_q_ << std::endl;
  out_file << "#    q    intensity ";
  if (experimental_) out_file << "   error";
  out_file << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < size(); i++) {
    if (max_q > 0 && q_[i] > max_q) break;
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(5);
    out_file << q_[i] << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << intensity_[i] << " ";

    if (experimental_) {  // do not print error for theoretical profiles
      out_file.setf(std::ios::left);
      out_file.width(10);
      out_file.precision(8);
      out_file << error_[i];
    }
    out_file << std::endl;
  }
  out_file.close();
}

void Profile::read_partial_profiles(const String& file_name) {
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  unsigned int psize = 6;
  // init
  partial_profiles_.insert(partial_profiles_.begin(), psize,
                           std::vector<double>());
  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line);  // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() != 7) continue;
    double q = atof(split_results[0].c_str());
    for (unsigned int i = 0; i < 6; i++) {
      partial_profiles_[i].push_back(atof(split_results[i + 1].c_str()));
    }
    // just add value for intensity. will be updated by sum_partial_profiles
    add_entry(q, 1.0);
  }
  in_file.close();

  sum_partial_profiles(1.0, 0.0);

  // determine qmin, qmax and delta
  if (size() > 1) {
    min_q_ = q_[0];
    max_q_ = q_[size() - 1];

    if (is_uniform_sampling()) {
      // To minimize rounding errors, by averaging differences of q
      Float diff = 0.0;
      for (unsigned int i = 1; i < size(); i++) diff += q_[i] - q_[i - 1];
      delta_q_ = diff / (size() - 1);
    } else {
      delta_q_ = (max_q_ - min_q_) / (size() - 1);
    }
  }

  IMP_LOG_TERSE("read_partial_profiles: "
                << file_name << " size= " << size() << " delta= " << delta_q_
                << " min_q= " << min_q_ << " max_q= " << max_q_ << std::endl);
}

void Profile::write_partial_profiles(const String& file_name) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  // header line
  out_file << "# SAXS profile: number of points = " << size()
           << ", q_min = " << min_q_ << ", q_max = " << max_q_;
  out_file << ", delta_q = " << delta_q_ << std::endl;
  out_file << "#    q    intensity ";
  out_file << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  for (unsigned int i = 0; i < size(); i++) {
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(5);
    out_file << q_[i] << " ";
    if (partial_profiles_.size() > 0) {
      for (unsigned int j = 0; j < partial_profiles_.size(); j++) {
        out_file.setf(std::ios::left);
        out_file.width(15);
        out_file.precision(8);
        out_file << partial_profiles_[j][i] << " ";
      }
    } else {  // not a partial profile
      out_file << intensity_[i] << " ";
      if (experimental_) {  // do not print error for theoretical profiles
        out_file.setf(std::ios::left);
        out_file.width(10);
        out_file.precision(8);
        out_file << error_[i];
      }
    }
    out_file << std::endl;
  }
  out_file.close();
}

void Profile::calculate_profile_real(const kernel::Particles& particles,
                                     FormFactorType ff_type) {
  IMP_LOG_TERSE("start real profile calculation for "
                << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist;  // fi(0) fj(0)
  // prepare coordinates and form factors in advance, for faster access
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);
  Floats form_factors;
  get_form_factors(particles, ff_table_, form_factors, ff_type);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      double prod = form_factors[i] * form_factors[j];
      r_dist.add_to_distribution(dist, 2 * prod);
    }
    // add autocorrelation part
    r_dist.add_to_distribution(0.0, square(form_factors[i]));
  }
  squared_distribution_2_profile(r_dist);
}

Float Profile::calculate_I0(const kernel::Particles& particles,
                            FormFactorType ff_type) {
  Float I0 = 0.0;
  for (unsigned int i = 0; i < particles.size(); i++)
    I0 += ff_table_->get_vacuum_form_factor(particles[i], ff_type);
  return square(I0);
}

void Profile::calculate_profile_constant_form_factor(
    const kernel::Particles& particles, Float form_factor) {
  IMP_LOG_TERSE("start real profile calculation for "
                << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist;
  // prepare coordinates and form factors in advance, for faster access
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);
  Float ff = square(form_factor);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      r_dist.add_to_distribution(dist, 2 * ff);
    }
    // add autocorrelation part
    r_dist.add_to_distribution(0.0, ff);
  }
  squared_distribution_2_profile(r_dist);
}

void Profile::calculate_profile_partial(const kernel::Particles& particles,
                                        const Floats& surface,
                                        FormFactorType ff_type) {
  IMP_LOG_TERSE("start real partial profile calculation for "
                << particles.size() << " particles " << std::endl);

  // copy coordinates and form factors in advance, to avoid n^2 copy
  // operations
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);
  Floats vacuum_ff(particles.size()), dummy_ff(particles.size()), water_ff;
  for (unsigned int i = 0; i < particles.size(); i++) {
    vacuum_ff[i] = ff_table_->get_vacuum_form_factor(particles[i], ff_type);
    dummy_ff[i] = ff_table_->get_dummy_form_factor(particles[i], ff_type);
  }
  if (surface.size() == particles.size()) {
    water_ff.resize(particles.size());
    Float ff_water = ff_table_->get_water_form_factor();
    for (unsigned int i = 0; i < particles.size(); i++) {
      water_ff[i] = surface[i] * ff_water;
    }
  }

  int r_size = 3;
  if (surface.size() == particles.size()) r_size = 6;
  std::vector<RadialDistributionFunction> r_dist(r_size);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      r_dist[0].add_to_distribution(
          dist, 2 * vacuum_ff[i] * vacuum_ff[j]);  // constant
      r_dist[1]
          .add_to_distribution(dist, 2 * dummy_ff[i] * dummy_ff[j]);  // c1^2
      r_dist[2]
          .add_to_distribution(dist, 2 * (vacuum_ff[i] * dummy_ff[j] +
                                          vacuum_ff[j] * dummy_ff[i]));  // -c1
      if (r_size > 3) {
        r_dist[3]
            .add_to_distribution(dist, 2 * water_ff[i] * water_ff[j]);  // c2^2
        r_dist[4]
            .add_to_distribution(dist, 2 * (vacuum_ff[i] * water_ff[j] +
                                            vacuum_ff[j] * water_ff[i]));  // c2
        r_dist[5].add_to_distribution(
            dist, 2 * (water_ff[i] * dummy_ff[j] +
                       water_ff[j] * dummy_ff[i]));  // -c1*c2
      }
    }
    // add autocorrelation part
    r_dist[0].add_to_distribution(0.0, square(vacuum_ff[i]));
    r_dist[1].add_to_distribution(0.0, square(dummy_ff[i]));
    r_dist[2].add_to_distribution(0.0, 2 * vacuum_ff[i] * dummy_ff[i]);
    if (r_size > 3) {
      r_dist[3].add_to_distribution(0.0, square(water_ff[i]));
      r_dist[4].add_to_distribution(0.0, 2 * vacuum_ff[i] * water_ff[i]);
      r_dist[5].add_to_distribution(0.0, 2 * water_ff[i] * dummy_ff[i]);
    }
  }

  // convert to reciprocal space
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           std::vector<double>());
  squared_distributions_2_partial_profiles(r_dist);

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0);
}

void Profile::calculate_profile_partial(const kernel::Particles& particles1,
                                        const kernel::Particles& particles2,
                                        const Floats& surface1,
                                        const Floats& surface2,
                                        FormFactorType ff_type) {
  IMP_LOG_TERSE("start real partial profile calculation for "
                << particles1.size() << " particles + " << particles2.size()
                << std::endl);

  // store coordinates
  std::vector<algebra::Vector3D> coordinates1, coordinates2;
  get_coordinates(particles1, coordinates1);
  get_coordinates(particles2, coordinates2);
  int r_size = 3;
  // get form factors
  Floats vacuum_ff1(particles1.size()), dummy_ff1(particles1.size()), water_ff1;
  for (unsigned int i = 0; i < particles1.size(); i++) {
    vacuum_ff1[i] = ff_table_->get_vacuum_form_factor(particles1[i], ff_type);
    dummy_ff1[i] = ff_table_->get_dummy_form_factor(particles1[i], ff_type);
  }
  Floats vacuum_ff2(particles2.size()), dummy_ff2(particles2.size()), water_ff2;
  for (unsigned int i = 0; i < particles2.size(); i++) {
    vacuum_ff2[i] = ff_table_->get_vacuum_form_factor(particles2[i], ff_type);
    dummy_ff2[i] = ff_table_->get_dummy_form_factor(particles2[i], ff_type);
  }
  // water layer
  if (surface1.size() > 0 && surface2.size() > 0 &&
      surface1.size() == particles1.size() &&
      surface2.size() == particles2.size()) {
    Float ff_water = ff_table_->get_water_form_factor();
    water_ff1.resize(particles1.size());
    water_ff2.resize(particles2.size());
    for (unsigned int i = 0; i < particles1.size(); i++) {
      water_ff1[i] = surface1[i] * ff_water;
    }
    for (unsigned int i = 0; i < particles2.size(); i++) {
      water_ff2[i] = surface2[i] * ff_water;
    }
    r_size = 6;
  }

  std::vector<RadialDistributionFunction> r_dist(r_size);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = get_squared_distance(coordinates1[i], coordinates2[j]);
      r_dist[0].add_to_distribution(
          dist, 2 * vacuum_ff1[i] * vacuum_ff2[j]);  // constant
      r_dist[1]
          .add_to_distribution(dist, 2 * dummy_ff1[i] * dummy_ff2[j]);  // c1^2
      r_dist[2].add_to_distribution(dist,
                                    2 * (vacuum_ff1[i] * dummy_ff2[j] +
                                         vacuum_ff2[j] * dummy_ff1[i]));  // -c1
      if (r_size > 3) {
        r_dist[3].add_to_distribution(dist,
                                      2 * water_ff1[i] * water_ff2[j]);  // c2^2
        r_dist[4].add_to_distribution(
            dist, 2 * (vacuum_ff1[i] * water_ff2[j] +
                       vacuum_ff2[j] * water_ff1[i]));  // c2
        r_dist[5].add_to_distribution(
            dist, 2 * (water_ff1[i] * dummy_ff2[j] +
                       water_ff2[j] * dummy_ff1[i]));  //-c1*c2
      }
    }
  }

  // convert to reciprocal space
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           std::vector<double>());
  squared_distributions_2_partial_profiles(r_dist);

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0);
}

void Profile::sum_partial_profiles(Float c1, Float c2) {
  // precomputed exp function
  static internal::ExpFunction ef(square(get_max_q()) * 0.3, 0.00001);

  if (partial_profiles_.size() > 0 &&
      (fabs(c1_ - c1) > 0.000001 || fabs(c2_ - c2) > 0.000001)) {
    // implements volume fitting function G(s) as described
    // in crysol paper eq. 13
    Float rm = average_radius_;
    // this exponent should match the exponent of g(s) which doesn't have
    // (4pi/3)^3/2 part so it seems that this part is not needed here too.
    // Float coefficient =
    // - std::pow((4.0*PI/3.0), 3.0/2.0) * square(rm) * (c1*c1-1.0) /
    // (4*PI);
    Float coefficient = -square(rm) * (c1 * c1 - 1.0) / (4 * PI);
    Float square_c2 = c2 * c2;
    Float cube_c1 = c1 * c1 * c1;

    if (size() != partial_profiles_[0].size()) init();

    for (unsigned int k = 0; k < size(); k++) {
      Float q = get_q(k);
      Float x = coefficient * square(q);
      Float G_q = cube_c1;
      if (std::abs(x) > 1.0e-8) G_q *= ef.exp(x);
      // Float G_q = cube_c1 * std::exp(coefficient*square(q));

      Float intensity = partial_profiles_[0][k];
      intensity += square(G_q) * partial_profiles_[1][k];
      intensity -= G_q * partial_profiles_[2][k];

      if (partial_profiles_.size() > 3) {
        intensity += square_c2 * partial_profiles_[3][k];
        intensity += c2 * partial_profiles_[4][k];
        intensity -= G_q * c2 * partial_profiles_[5][k];
      }
      set_intensity(k, intensity);
    }
    // set new c1/c2 values
    c1_ = c1;
    c2_ = c2;
  }
}

void Profile::resample(const Profile* exp_profile, Profile* resampled_profile,
                       bool partial_profiles) const {
  if (q_mapping_.size() == 0)
    for (unsigned int k = 0; k < size(); k++)
      const_cast<Profile*>(this)->q_mapping_[q_[k]] = k;

  if (partial_profiles) {
    // init
    resampled_profile->partial_profiles_.insert(
        resampled_profile->partial_profiles_.begin(), partial_profiles_.size(),
        std::vector<double>());
  }

  for (unsigned int k = 0; k < exp_profile->size(); k++) {
    Float q = exp_profile->get_q(k);
    if (q > max_q_) break;

    std::map<float, unsigned int>::const_iterator it =
        q_mapping_.lower_bound(q);
    if (it == q_mapping_.end()) break;

    unsigned int i = it->second;
    Float delta_q = 1.0;
    if (i == 0 || (delta_q = get_q(i) - get_q(i - 1)) <= 1.0e-16) {
      if (partial_profiles) {
        for (unsigned int r = 0; r < partial_profiles_.size(); r++) {
          resampled_profile->partial_profiles_[r]
              .push_back(partial_profiles_[r][i]);
        }
      }
      resampled_profile->add_entry(q, get_intensity(i));
    } else {
      // interpolate
      Float alpha = (q - get_q(i - 1)) / delta_q;
      if (alpha > 1.0) alpha = 1.0;  // handle rounding errors

      if (partial_profiles) {
        for (unsigned int r = 0; r < partial_profiles_.size(); r++) {
          Float intensity =
              partial_profiles_[r][i - 1] +
              alpha * (partial_profiles_[r][i] - partial_profiles_[r][i - 1]);
          resampled_profile->partial_profiles_[r].push_back(intensity);
        }
      }
      Float intensity = get_intensity(i - 1) +
                        alpha * (get_intensity(i) - get_intensity(i - 1));
      resampled_profile->add_entry(q, intensity);
    }
  }
}

void Profile::downsample(Profile* downsampled_profile,
                         unsigned int point_number) const {

  unsigned int points_delta = (int)std::ceil(size() / (float)point_number);
  for (unsigned int k = 0; k < size(); k += points_delta)
    downsampled_profile->add_entry(get_q(k), get_intensity(k), get_error(k));
}

void Profile::calculate_profile_symmetric(const kernel::Particles& particles,
                                          unsigned int n,
                                          FormFactorType ff_type) {
  IMP_USAGE_CHECK(n > 1,
                  "Attempting to use symmetric computation, symmetry order"
                      << " should be > 1. Got: " << n);
  IMP_LOG_TERSE("start real profile calculation for "
                << particles.size() << " particles with symmetry = " << n
                << std::endl);
  // split units, only number_of_distances units is needed
  unsigned int number_of_distances = n / 2;
  unsigned int unit_size = particles.size() / n;
  // coordinates
  std::vector<std::vector<algebra::Vector3D> > units(
      number_of_distances + 1, std::vector<algebra::Vector3D>(unit_size));
  for (unsigned int i = 0; i <= number_of_distances; i++) {
    for (unsigned int j = 0; j < unit_size; j++) {
      units[i][j] = core::XYZ(particles[i * unit_size + j]).get_coordinates();
    }
  }
  Floats form_factors(unit_size);
  for (unsigned int i = 0; i < unit_size; i++) {
    form_factors[i] = ff_table_->get_form_factor(particles[i], ff_type);
  }

  RadialDistributionFunction r_dist;
  // distribution within unit
  for (unsigned int i = 0; i < unit_size; i++) {
    for (unsigned int j = i + 1; j < unit_size; j++) {
      Float dist2 = get_squared_distance(units[0][i], units[0][j]);
      r_dist.add_to_distribution(dist2, 2 * form_factors[i] * form_factors[j]);
    }
    r_dist.add_to_distribution(0.0, square(form_factors[i]));
  }

  // distributions between units separated by distance i
  for (unsigned int in = 1; in < number_of_distances; in++) {
    for (unsigned int i = 0; i < unit_size; i++) {
      for (unsigned int j = 0; j < unit_size; j++) {
        Float dist2 = get_squared_distance(units[0][i], units[in][j]);
        r_dist.add_to_distribution(dist2,
                                   2 * form_factors[i] * form_factors[j]);
      }
    }
  }
  r_dist.scale(n);

  // distribution between units separated by distance n/2
  RadialDistributionFunction r_dist2;
  for (unsigned int i = 0; i < unit_size; i++) {
    for (unsigned int j = 0; j < unit_size; j++) {
      Float dist2 =
          get_squared_distance(units[0][i], units[number_of_distances][j]);
      r_dist2.add_to_distribution(dist2, 2 * form_factors[i] * form_factors[j]);
    }
  }

  // if n is even, the scale is by n/2
  // if n is odd the scale is by n
  if (n & 1)
    r_dist2.scale(n);  // odd
  else
    r_dist2.scale(n / 2);  // even
  r_dist2.add(r_dist);

  squared_distribution_2_profile(r_dist2);
}

void Profile::calculate_profile_real(const kernel::Particles& particles1,
                                     const kernel::Particles& particles2,
                                     FormFactorType ff_type) {
  IMP_LOG_TERSE("start real profile calculation for "
                << particles1.size() << " + " << particles2.size()
                << " particles" << std::endl);
  RadialDistributionFunction r_dist;  // fi(0) fj(0)

  // copy coordinates and form factors in advance, to avoid n^2 copy
  // operations
  std::vector<algebra::Vector3D> coordinates1, coordinates2;
  get_coordinates(particles1, coordinates1);
  get_coordinates(particles2, coordinates2);
  Floats form_factors1, form_factors2;
  get_form_factors(particles1, ff_table_, form_factors1, ff_type);
  get_form_factors(particles2, ff_table_, form_factors2, ff_type);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = get_squared_distance(coordinates1[i], coordinates2[j]);
      double prod = form_factors1[i] * form_factors2[j];
      r_dist.add_to_distribution(dist, 2 * prod);
    }
  }
  squared_distribution_2_profile(r_dist);
}

void Profile::distribution_2_profile(const RadialDistributionFunction& r_dist) {
  init();
  // iterate over intensity profile
  for (unsigned int k = 0; k < size(); k++) {
    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.size(); r++) {
      Float dist = r_dist.index2dist(r);
      Float x = dist * q_[k];
      x = boost::math::sinc_pi(x);
      intensity_[k] += r_dist[r] * x;
    }
  }
}

void Profile::squared_distribution_2_profile(
    const RadialDistributionFunction& r_dist) {
  init();
  // precomputed sin(x)/x function
  static internal::SincFunction sf(
      sqrt(r_dist.get_max_distance()) * get_max_q(), 0.0001);

  // precompute square roots of distances
  std::vector<float> distances(r_dist.size(), 0.0);
  for (unsigned int r = 0; r < r_dist.size(); r++)
    if (r_dist[r] != 0.0) distances[r] = sqrt(r_dist.index2dist(r));

  // iterate over intensity profile
  for (unsigned int k = 0; k < size(); k++) {
    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.size(); r++) {
      if (r_dist[r] != 0.0) {
        // x = sin(dq)/dq
        float dist = distances[r];
        float x = dist * q_[k];
        x = sf.sinc(x);
        // multiply by the value from distribution
        intensity_[k] += r_dist[r] * x;
      }
    }
    // this correction is required since we approximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    intensity_[k] *= std::exp(-modulation_function_parameter_ * square(q_[k]));
  }
}

void Profile::squared_distributions_2_partial_profiles(
    const std::vector<RadialDistributionFunction>& r_dist) {
  int r_size = r_dist.size();

  // init
  init();
  for (int i = 0; i < r_size; i++)
    partial_profiles_[i].resize(intensity_.size());

  // precomputed sin(x)/x function
  static internal::SincFunction sf(
      sqrt(r_dist[0].get_max_distance()) * get_max_q(), 0.0001);

  // precompute square roots of distances
  std::vector<float> distances(r_dist[0].size(), 0.0);
  for (unsigned int r = 0; r < r_dist[0].size(); r++)
    if (r_dist[0][r] > 0.0) distances[r] = sqrt(r_dist[0].index2dist(r));

  // iterate over intensity profile
  for (unsigned int k = 0; k < q_.size(); k++) {
    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist[0].size(); r++) {
      // x = sin(dq)/dq
      Float dist = distances[r];
      Float x = dist * q_[k];
      x = sf.sinc(x);
      // iterate over partial profiles
      if (r_dist[0][r] > 0.0) {
        for (int i = 0; i < r_size; i++) {
          // multiply by the value from distribution
          partial_profiles_[i][k] += r_dist[i][r] * x;
        }
      }
    }
    // this correction is required since we approximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    Float corr = std::exp(-modulation_function_parameter_ * square(q_[k]));
    for (int i = 0; i < r_size; i++) partial_profiles_[i][k] *= corr;
  }
}

void Profile::add(const Profile* other_profile, Float weight) {
  if (size() == 0 && other_profile->size() != 0) {
    min_q_ = other_profile->get_min_q();
    max_q_ = other_profile->get_max_q();
    delta_q_ = other_profile->get_delta_q();
    init();
  }
  // assumes same q values!!!
  for (unsigned int k = 0; k < size(); k++) {
    intensity_[k] += weight * other_profile->intensity_[k];
  }
}

void Profile::add(const std::vector<Profile*>& profiles,
                  const std::vector<Float>& weights) {
  for (unsigned int i = 0; i < profiles.size(); i++) {
    Float weight = 1.0;
    if (weights.size() > i) weight = weights[i];
    add(profiles[i], weight);
  }
}

void Profile::add_partial_profiles(const Profile* other_profile, Float weight) {
  if (other_profile->partial_profiles_.size() > 0 &&
      partial_profiles_.size() == 0) {
    partial_profiles_.insert(partial_profiles_.begin(),
                             other_profile->partial_profiles_.size(),
                             std::vector<double>());
  }
  if (partial_profiles_.size() != other_profile->partial_profiles_.size()) {
    IMP_WARN("Can't add different partial profile sizes "
             << partial_profiles_.size() << "-"
             << other_profile->partial_profiles_.size() << std::endl);
    return;
  }
  for (unsigned int i = 0; i < partial_profiles_.size(); i++) {
    for (unsigned int j = 0; j < partial_profiles_[i].size(); j++) {
      partial_profiles_[i][j] +=
          weight * other_profile->partial_profiles_[i][j];
    }
  }
}

void Profile::add_partial_profiles(const std::vector<Profile*>& profiles,
                                   const std::vector<Float>& weights) {
  for (unsigned int i = 0; i < profiles.size(); i++) {
    Float weight = 1.0;
    if (weights.size() > i) weight = weights[i];
    add_partial_profiles(profiles[i], weight);
  }
}

double Profile::radius_of_gyration_fixed_q(double end_q) const {
  IMP::algebra::Vector2Ds data;  // x=q^2, y=logI(q)) z=error(q)/I(q)
  Floats errors;
  for (unsigned int i = 0; i < size(); i++) {
    double q = q_[i];
    double Iq = intensity_[i];
    double err = error_[i] / Iq;
    double logIq = log(Iq);
    if (q > end_q) break;
    algebra::Vector2D v(q * q, logIq);
    data.push_back(v);
    errors.push_back(err);
  }

  algebra::LinearFit2D lf(data, errors);
  double a = lf.get_a();
  if (a >= 0) return 0.0;
  double rg = sqrt(-3 * a);
  return rg;
}

double Profile::radius_of_gyration(double end_q_rg) const {
  double qlimit = min_q_ + delta_q_ * 5;  // start after 5 points
  for (double q = qlimit; q < max_q_; q += delta_q_) {
    double rg = radius_of_gyration_fixed_q(q);
    if (rg > 0.0) {
      if (q * rg < end_q_rg)
        qlimit = q;
      else
        break;
    }
  }
  double rg = radius_of_gyration_fixed_q(qlimit);
  return rg;
}

void Profile::background_adjust(double start_q) {
  algebra::Vector2Ds data;  // x=q^2, y=sum(q^2xI(q))
  double sum = 0.0;
  for (unsigned int i = 0; i < size(); i++) {
    double q = q_[i];
    double Iq = intensity_[i];
    double q2xIq = q * q * Iq;
    sum += q2xIq;
    if (q >= start_q) {
      algebra::Vector2D v(q * q, sum);
      data.push_back(v);
    }
  }

  algebra::ParabolicFit p(data);
  double P3 = p.get_a();
  double P2 = p.get_b();
  double P1 = p.get_c();
  double G1 = P2 / P1;
  double G2 = 12.0 * (P3 / P1 - G1 * G1 / 4.0);

  for (unsigned int i = 0; i < size(); i++) {
    double q = q_[i];
    double q2 = q * q;
    double q4 = q2 * q2;
    double Iq = intensity_[i];
    double Iq_new = Iq / (1.0 + q2 * G1 + q4 * (G1 * G1 / 4.0 + G2 / 12.0));
    intensity_[i] = Iq_new;
  }
}

void Profile::scale(Float c) {
  for (unsigned int k = 0; k < size(); k++) intensity_[k] *= c;
}

void Profile::offset(Float c) {
  for (unsigned int k = 0; k < size(); k++) intensity_[k] -= c;
}

void Profile::copy_errors(const Profile* exp_profile) {
  if (size() != exp_profile->size()) {
    IMP_THROW("Profile::copy_errors is supported "
                  << "only for profiles with the same q values!",
              ValueException);
  }
  // assumes same q values!!!
  error_ = exp_profile->error_;
}

void Profile::profile_2_distribution(RadialDistributionFunction& rd,
                                     Float max_distance) const {
  float scale = 1.0 / (2 * PI * PI);
  unsigned int distribution_size = rd.dist2index(max_distance) + 1;

  // offset profile so that minimal i(q) is zero
  float min_value = intensity_[0];
  for (unsigned int k = 0; k < size(); k++) {
    if (intensity_[k] < min_value) min_value = intensity_[k];
  }
  Profile p(min_q_, max_q_, delta_q_);
  p.init();
  for (unsigned int k = 0; k < size(); k++) {
    p.intensity_[k] = intensity_[k] - min_value;
  }

  // iterate over r
  for (unsigned int i = 0; i < distribution_size; i++) {
    Float r = rd.index2dist(i);
    Float sum = 0.0;
    // sum over q: SUM (I(q)*q*sin(qr))
    for (unsigned int k = 0; k < p.size(); k++) {
      sum += p.intensity_[k] * p.q_[k] * std::sin(p.q_[k] * r);
    }
    rd.add_to_distribution(r, r * scale * sum);
  }
}

void Profile::calculate_profile_reciprocal(const kernel::Particles& particles,
                                           FormFactorType ff_type) {
  if (ff_type == CA_ATOMS) {
    IMP_WARN("Reciprocal space profile calculation is not suported for"
             << "residue level" << std::endl);
    return;
  }

  IMP_LOG_TERSE("start reciprocal profile calculation for "
                << particles.size() << " particles" << std::endl);
  init();
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);

  // iterate over pairs of atoms
  // loop1
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    const Floats& factors1 = ff_table_->get_form_factors(particles[i], ff_type);
    // loop2
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      const Floats& factors2 =
          ff_table_->get_form_factors(particles[j], ff_type);
      Float dist = get_distance(coordinates[i], coordinates[j]);
      // loop 3
      // iterate over intensity profile
      for (unsigned int k = 0; k < size(); k++) {
        Float x = dist * q_[k];
        x = boost::math::sinc_pi(x);
        intensity_[k] += 2 * x * factors1[k] * factors2[k];
      }  // end of loop 3
    }    // end of loop 2

    // add autocorrelation part
    for (unsigned int k = 0; k < size(); k++) {
      intensity_[k] += factors1[k] * factors1[k];
    }
  }  // end of loop1
}

void Profile::calculate_profile_reciprocal_partial(
    const kernel::Particles& particles, const Floats& surface,
    FormFactorType ff_type) {
  if (ff_type == CA_ATOMS) {
    IMP_WARN("Reciprocal space profile calculation is not suported for"
             << "residue level" << std::endl);
    return;
  }

  IMP_LOG_TERSE("start partial reciprocal profile calculation for "
                << particles.size() << " particles" << std::endl);

  init();
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);

  // allocate partial profiles
  int r_size = 3;
  if (surface.size() == particles.size()) r_size = 6;
  const Floats& water_ff = ff_table_->get_water_form_factors();
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           std::vector<double>());
  for (int i = 0; i < r_size; i++)
    partial_profiles_[i].resize(intensity_.size());

  // iterate over pairs of atoms
  // loop1
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    const Floats& vacuum_ff1 =
        ff_table_->get_vacuum_form_factors(particles[i], ff_type);
    const Floats& dummy_ff1 =
        ff_table_->get_dummy_form_factors(particles[i], ff_type);
    // loop2
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      const Floats& vacuum_ff2 =
          ff_table_->get_vacuum_form_factors(particles[j], ff_type);
      const Floats& dummy_ff2 =
          ff_table_->get_dummy_form_factors(particles[j], ff_type);
      Float dist = get_distance(coordinates[i], coordinates[j]);

      // loop 3
      // iterate over intensity profile
      for (unsigned int k = 0; k < size(); k++) {
        Float x = dist * q_[k];
        x = 2 * boost::math::sinc_pi(x);
        partial_profiles_[0][k] += x * vacuum_ff1[k] * vacuum_ff2[k];
        partial_profiles_[1][k] += x * dummy_ff1[k] * dummy_ff2[k];
        partial_profiles_[2][k] +=
            x * (vacuum_ff1[k] * dummy_ff2[k] + vacuum_ff2[k] * dummy_ff1[k]);

        if (r_size > 3) {
          partial_profiles_[3][k] +=
              x * surface[i] * surface[j] * water_ff[k] * water_ff[k];
          partial_profiles_[4][k] +=
              x * (vacuum_ff1[k] * surface[j] * water_ff[k] +
                   vacuum_ff2[k] * surface[i] * water_ff[k]);
          partial_profiles_[5][k] +=
              x * (dummy_ff1[k] * surface[j] * water_ff[k] +
                   dummy_ff2[k] * surface[i] * water_ff[k]);
        }
      }  // end of loop 3
    }    // end of loop 2

    // add autocorrelation part
    for (unsigned int k = 0; k < size(); k++) {
      partial_profiles_[0][k] += vacuum_ff1[k] * vacuum_ff1[k];
      partial_profiles_[1][k] += dummy_ff1[k] * dummy_ff1[k];
      partial_profiles_[2][k] += 2 * vacuum_ff1[k] * dummy_ff1[k];

      if (r_size > 3) {
        partial_profiles_[3][k] += square(surface[i] * water_ff[k]);
        partial_profiles_[4][k] += 2 * vacuum_ff1[k] * surface[i] * water_ff[k];
        partial_profiles_[5][k] += 2 * dummy_ff1[k] * surface[i] * water_ff[k];
      }
    }
  }  // end of loop1

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0);
}

IMPSAXS_END_NAMESPACE

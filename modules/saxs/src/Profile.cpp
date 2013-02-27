/**
 *  \file Profile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/utility.h>
#include <IMP/saxs/internal/sinc_function.h>
#include <IMP/saxs/internal/variance_helpers.h>

#include <IMP/base/math.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ParabolicFit.h>
#include <IMP/algebra/LinearFit.h>
#include <IMP/constants.h>

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/math/special_functions/sinc.hpp>

#include <fstream>
#include <string>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

const Float Profile::modulation_function_parameter_ = 0.23;

std::ostream & operator<<(std::ostream & s, const Profile::IntensityEntry & e)
{
  return s << e.q_ << " " << e.intensity_ << " " << e.error_ << std::endl;
}

Profile::Profile(Float qmin, Float qmax, Float delta):
  min_q_(qmin), max_q_(qmax), delta_q_(delta), experimental_(false),
  average_radius_(1.58), average_volume_(17.5)
{
  ff_table_ = default_form_factor_table();
}

Profile::Profile(const String& file_name) : experimental_(true)
{
  read_SAXS_file(file_name);
}

void Profile::init(bool variance)
{
  profile_.clear();
  int number_of_q_entries = (int)std::ceil((max_q_ - min_q_) / delta_q_ )+1;

  for (int i=0; i<number_of_q_entries; i++) {
    IntensityEntry entry(min_q_ + i * delta_q_);
    profile_.push_back(entry);
    if(variance) {
      variances_.push_back(std::vector<double>((number_of_q_entries-i), 0.0));
    }
  }
}

void Profile::read_SAXS_file(const String& file_name)
{
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  bool with_error = false;
  std::string line;
  IntensityEntry entry;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector < std::string > split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() < 2 || split_results.size() > 5) continue;
    entry.q_ = atof(split_results[0].c_str());
    entry.intensity_ = atof(split_results[1].c_str());

    // validity checks
    if(fabs(entry.intensity_) < IMP_SAXS_DELTA_LIMIT)
      continue;// skip zero intensities
    if(entry.intensity_ < 0.0) { // negative intensity
      IMP_WARN("Negative intensity value: " << line
               << " skipping remaining profile points" << std::endl);
      break;
    }
    if (split_results.size() >= 3) {
      entry.error_ = atof(split_results[2].c_str());
      if(fabs(entry.error_) < IMP_SAXS_DELTA_LIMIT) {
        entry.error_ = 0.05 * entry.intensity_;
        if(fabs(entry.error_) < IMP_SAXS_DELTA_LIMIT) continue; //skip entry
      }
      with_error = true;
    }
    profile_.push_back(entry);
  }
  in_file.close();

  // determine qmin, qmax and delta
  if (profile_.size() > 1) {
    min_q_ = profile_[0].q_;
    max_q_ = profile_[profile_.size() - 1].q_;

    if(is_uniform_sampling()) {
      // To minimize rounding errors, by averaging differences of q
      Float diff = 0.0;
      for (unsigned int i=1; i<profile_.size(); i++) {
        diff += profile_[i].q_ - profile_[i-1].q_;
      }
      delta_q_ = diff / (profile_.size()-1);
    } else {
      delta_q_ = (max_q_ - min_q_)/(profile_.size()-1);
    }
  }

  IMP_LOG_TERSE( "read_SAXS_file: " << file_name
          << " size= " << profile_.size() << " delta= " << delta_q_
          << " min_q= " << min_q_ << " max_q= " << max_q_ << std::endl);

  // saxs_read: No experimental error specified, add errors
  if (!with_error) {
    add_errors();
    IMP_LOG_TERSE( "read_SAXS_file: No experimental error specified"
            << " -> error added " << std::endl);
  }
}

void Profile::add_errors() {
  // init random number generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type rng;
  //rng.seed(static_cast<unsigned int>(std::time(0)));

  // init distribution
  typedef boost::poisson_distribution< > poisson;
  poisson poisson_dist(10.0);
  typedef boost::variate_generator<base_generator_type&, poisson>
    poisson_generator_type;
  poisson_generator_type poisson_rng(rng, poisson_dist);

  for(unsigned int i=0; i<profile_.size(); i++) {
    double ra = std::abs(poisson_rng()/10.0 - 1.0) + 1.0;
    //std::cerr << "I " << profile_[i].intensity_ << "rand " << ra << std::endl;
    // 3% of error, scaled by 5q + poisson distribution
    profile_[i].error_ =
      0.03 * profile_[i].intensity_ * 5.0*(profile_[i].q_+0.001) * ra;
  }
}

void Profile::add_noise(Float percentage) {
  // init random number generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type rng;

  // init distribution
  typedef boost::poisson_distribution< > poisson;
  poisson poisson_dist(10.0);
  typedef boost::variate_generator<base_generator_type&, poisson>
    poisson_generator_type;
  poisson_generator_type poisson_rng(rng, poisson_dist);

  for(unsigned int i=0; i<profile_.size(); i++) {
    double random_number = poisson_rng()/10.0 - 1.0;
    // X% of intensity weighted by (1+q) + poisson distribution
    profile_[i].intensity_ +=
      percentage * profile_[i].intensity_ *(profile_[i].q_+1.0) * random_number;
  }
}

bool Profile::is_uniform_sampling() const {
  if (profile_.size() <= 1) return false;

  Float curr_diff = profile_[1].q_ - profile_[0].q_;
  Float epsilon = curr_diff / 10;
  for (unsigned int i=2; i<profile_.size(); i++) {
    Float diff = profile_[i].q_ - profile_[i-1].q_;
    if(fabs(curr_diff - diff) > epsilon) return false;
  }
  return true;
}

void Profile::write_SAXS_file(const String& file_name, Float max_q) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  // header line
  out_file << "# SAXS profile: number of points = " << profile_.size()
           << ", q_min = " << min_q_ << ", q_max = ";
  if(max_q > 0) out_file << max_q;
  else out_file << max_q_;
  out_file << ", delta_q = " << delta_q_ << std::endl;
  out_file << "#    q    intensity ";
  if(experimental_) out_file << "   error";
  out_file << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < profile_.size(); i++) {
    if(max_q > 0 && profile_[i].q_ > max_q) break;
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(5);
    out_file << profile_[i].q_ << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << profile_[i].intensity_ << " ";

    if(experimental_) { // do not print error for theoretical profiles
      out_file.setf(std::ios::left);
      out_file.width(10);
      out_file.precision(8);
      out_file << profile_[i].error_;
    }
    out_file << std::endl;
  }
  out_file.close();
}

void Profile::write_partial_profiles(const String& file_name) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  // header line
  out_file << "# SAXS profile: number of points = " << profile_.size()
           << ", q_min = " << min_q_ << ", q_max = " << max_q_;
  out_file << ", delta_q = " << delta_q_ << std::endl;
  out_file << "#    q    intensity ";
  out_file << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  for (unsigned int i = 0; i < profile_.size(); i++) {
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(5);
    out_file << profile_[i].q_ << " ";
    for(unsigned int j=0; j<partial_profiles_.size(); j++) {
      out_file.setf(std::ios::left);
      out_file.width(15);
      out_file.precision(8);
      out_file << partial_profiles_[j].profile_[i].intensity_ << " ";
    }
    out_file << std::endl;
  }
  out_file.close();
}

void Profile::calculate_profile_real(const Particles& particles,
                                     FormFactorType ff_type,
                                     bool variance, double variance_tau)
{
  IMP_LOG_TERSE( "start real profile calculation for "
          << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist; //fi(0) fj(0)
  RadialDistributionFunction r_dist2; //fi(0)^2 fj(0)^2
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
      r_dist2.add_to_distribution(dist, 2 * prod * prod);
    }
    // add autocorrelation part
    r_dist.add_to_distribution(0.0,square(form_factors[i]));
    if (variance) r_dist2.add_to_distribution(0.0,square(
                                                square(form_factors[i])));
  }
  squared_distribution_2_profile(r_dist, r_dist2, variance, variance_tau);
}

Float Profile::calculate_I0(const Particles& particles, FormFactorType ff_type)
{
  Float I0=0.0;
  for(unsigned int i=0; i<particles.size(); i++)
    I0+= ff_table_->get_vacuum_form_factor(particles[i], ff_type);
  return square(I0);
}

void Profile::calculate_profile_constant_form_factor(const Particles& particles,
                                                     Float form_factor)
{
  IMP_LOG_TERSE( "start real profile calculation for "
          << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist, r_dist2;
  // prepare coordinates and form factors in advance, for faster access
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);
  Float ff = square(form_factor);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      r_dist.add_to_distribution(dist, 2*ff);
    }
    // add autocorrelation part
    r_dist.add_to_distribution(0.0, ff);
  }
  squared_distribution_2_profile(r_dist, r_dist2);
}

void Profile::calculate_profile_partial(const Particles& particles,
                                        const Floats& surface,
                                        FormFactorType ff_type)
{
  IMP_LOG_TERSE( "start real partial profile calculation for "
          << particles.size() << " particles " <<  std::endl);

  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);
  Floats vacuum_ff(particles.size()), dummy_ff(particles.size()), water_ff;
  for (unsigned int i=0; i<particles.size(); i++) {
    vacuum_ff[i] = ff_table_->get_vacuum_form_factor(particles[i], ff_type);
    dummy_ff[i] = ff_table_->get_dummy_form_factor(particles[i], ff_type);
  }
  if(surface.size() == particles.size()) {
    water_ff.resize(particles.size());
    Float ff_water = ff_table_->get_water_form_factor();
    for (unsigned int i=0; i<particles.size(); i++) {
      water_ff[i] = surface[i]*ff_water;
    }
  }

  int r_size = 3;
  if(surface.size() == particles.size()) r_size = 6;
  std::vector<RadialDistributionFunction> r_dist(r_size);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      r_dist[0].add_to_distribution(dist,
                                    2*vacuum_ff[i] * vacuum_ff[j]); // constant
      r_dist[1].add_to_distribution(dist, 2*dummy_ff[i] * dummy_ff[j]); // c1^2
      r_dist[2].add_to_distribution(dist, 2*(vacuum_ff[i] * dummy_ff[j]
                                         + vacuum_ff[j] * dummy_ff[i])); // -c1
      if(r_size > 3) {
        r_dist[3].add_to_distribution(dist, 2*water_ff[i]*water_ff[j]); // c2^2
        r_dist[4].add_to_distribution(dist, 2*(vacuum_ff[i] * water_ff[j]
                                          + vacuum_ff[j] * water_ff[i])); // c2
        r_dist[5].add_to_distribution(dist, 2*(water_ff[i] * dummy_ff[j]
                                       + water_ff[j] * dummy_ff[i])); // -c1*c2
      }
    }
    // add autocorrelation part
    r_dist[0].add_to_distribution(0.0, square(vacuum_ff[i]));
    r_dist[1].add_to_distribution(0.0, square(dummy_ff[i]));
    r_dist[2].add_to_distribution(0.0, 2*vacuum_ff[i] * dummy_ff[i]);
    if(r_size > 3) {
      r_dist[3].add_to_distribution(0.0, square(water_ff[i]));
      r_dist[4].add_to_distribution(0.0, 2*vacuum_ff[i] * water_ff[i]);
      r_dist[5].add_to_distribution(0.0, 2*water_ff[i] * dummy_ff[i]);
    }
  }

  // convert to reciprocal space
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           Profile(min_q_, max_q_, delta_q_));
  squared_distributions_2_partial_profiles(r_dist);

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0, *this);
}

void Profile::calculate_profile_partial(const Particles& particles1,
                                        const Particles& particles2,
                                        const Floats& surface1,
                                        const Floats& surface2,
                                        FormFactorType ff_type)
{
  IMP_LOG_TERSE( "start real partial profile calculation for "
          << particles1.size() << " particles + "
          << particles2.size() <<  std::endl);

  // store coordinates
  std::vector <algebra::Vector3D> coordinates1,coordinates2;
  get_coordinates(particles1, coordinates1);
  get_coordinates(particles2, coordinates2);
  int r_size = 3;
  // get form factors
  Floats vacuum_ff1(particles1.size()), dummy_ff1(particles1.size()), water_ff1;
  for(unsigned int i=0; i<particles1.size(); i++) {
    vacuum_ff1[i] = ff_table_->get_vacuum_form_factor(particles1[i], ff_type);
    dummy_ff1[i] = ff_table_->get_dummy_form_factor(particles1[i], ff_type);
  }
  Floats vacuum_ff2(particles2.size()), dummy_ff2(particles2.size()), water_ff2;
  for(unsigned int i=0; i<particles2.size(); i++) {
    vacuum_ff2[i] = ff_table_->get_vacuum_form_factor(particles2[i], ff_type);
    dummy_ff2[i] = ff_table_->get_dummy_form_factor(particles2[i], ff_type);
  }
  // water layer
  if(surface1.size() > 0 && surface2.size() > 0 &&
     surface1.size() == particles1.size() &&
     surface2.size() == particles2.size()) {
    Float ff_water = ff_table_->get_water_form_factor();
    water_ff1.resize(particles1.size());
    water_ff2.resize(particles2.size());
    for(unsigned int i=0; i<particles1.size(); i++) {
      water_ff1[i] = surface1[i]*ff_water;
    }
    for(unsigned int i=0; i<particles2.size(); i++) {
      water_ff2[i] = surface2[i]*ff_water;
    }
    r_size = 6;
  }

  std::vector<RadialDistributionFunction> r_dist(r_size);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = get_squared_distance(coordinates1[i], coordinates2[j]);
      r_dist[0].add_to_distribution(dist,
                           2*vacuum_ff1[i]*vacuum_ff2[j]); // constant
      r_dist[1].add_to_distribution(dist, 2*dummy_ff1[i]*dummy_ff2[j]);  // c1^2
      r_dist[2].add_to_distribution(dist, 2*(vacuum_ff1[i] * dummy_ff2[j]
                                        + vacuum_ff2[j] * dummy_ff1[i])); // -c1
      if(r_size > 3) {
        r_dist[3].add_to_distribution(dist, 2*water_ff1[i]*water_ff2[j]); //c2^2
        r_dist[4].add_to_distribution(dist, 2*(vacuum_ff1[i] * water_ff2[j]
                                        + vacuum_ff2[j] * water_ff1[i])); // c2
        r_dist[5].add_to_distribution(dist, 2*(water_ff1[i] * dummy_ff2[j]
                                       + water_ff2[j] * dummy_ff1[i])); //-c1*c2
      }
    }
  }

  // convert to reciprocal space
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           Profile(min_q_, max_q_, delta_q_));
  squared_distributions_2_partial_profiles(r_dist);

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0, *this);
}

void Profile::sum_partial_profiles(Float c1, Float c2, Profile& out_profile) {
  // implements volume fitting function G(s) as described in crysol paper eq. 13
  Float rm = average_radius_;
  // this exponent should match the exponent of g(s) which doesn't have
  // (4pi/3)^3/2 part so it seems that this part is not needed here too.
  //Float coefficient =
  // - std::pow((4.0*PI/3.0), 3.0/2.0) * square(rm) * (c1*c1-1.0) / (4*PI);
  Float coefficient = -square(rm) * (c1*c1-1.0) / (4*PI);

  //std::cerr << "coefficient " << coefficient << " c1 " << c1 << std::endl;
  if(partial_profiles_.size() > 0) {
    out_profile.init();
    out_profile.add(partial_profiles_[0]);
    Profile p1, p2;
    p1.add(partial_profiles_[1]);
    p2.add(partial_profiles_[2]);
    for(unsigned int k=0; k<p1.size(); k++) {
      Float q = p1.get_q(k);
      Float G_q = (c1*c1*c1) * std::exp(coefficient*square(q));
      p1.set_intensity(k, p1.get_intensity(k)*square(G_q));
      p2.set_intensity(k, - p2.get_intensity(k)*G_q);
      //if(k==p1.size()-1) std::cerr << q << " " << G_q << std::endl;
    }
    //p1.scale(c1*c1);
    //p2.scale(-c1);
    out_profile.add(p1);
    out_profile.add(p2);
  }
  if(partial_profiles_.size() > 3) {
    Profile p3, p4, p5;
    p3.add(partial_profiles_[3]);
    p4.add(partial_profiles_[4]);
    p5.add(partial_profiles_[5]);
    p3.scale(c2*c2);
    p4.scale(c2);
    for(unsigned int k=0; k<p5.size(); k++) {
      Float q = p5.get_q(k);
      Float G_q = (c1*c1*c1)*std::exp(coefficient*square(q));
      p5.set_intensity(k, - p5.get_intensity(k)*G_q*c2);
    }
    //    p5.scale(-c1*c2);
    out_profile.add(p3);
    out_profile.add(p4);
    out_profile.add(p5);
  }
}

void Profile::calculate_profile_symmetric(const Particles& particles,
                                          unsigned int n,
                                          FormFactorType ff_type)
{
  IMP_USAGE_CHECK(n > 1,
                  "Attempting to use symmetric computation, symmetry order"
                  << " should be > 1. Got: " << n);
  IMP_LOG_TERSE( "start real profile calculation for " << particles.size()
          << " particles with symmetry = " << n << std::endl);
  // split units, only number_of_distances units is needed
  unsigned int number_of_distances = n/2;
  unsigned int unit_size = particles.size()/n;
  // coordinates
  std::vector<std::vector<algebra::Vector3D> > units(number_of_distances+1,
                                 std::vector<algebra::Vector3D>(unit_size));
  for(unsigned int i=0; i<=number_of_distances; i++) {
    for(unsigned int j=0; j<unit_size; j++) {
      units[i][j] = core::XYZ(particles[i*unit_size+j]).get_coordinates();
    }
  }
  Floats form_factors(unit_size);
  for (unsigned int i=0; i<unit_size; i++) {
    form_factors[i] = ff_table_->get_form_factor(particles[i], ff_type);
  }

  RadialDistributionFunction r_dist;
  // distribution within unit
  for (unsigned int i=0; i<unit_size; i++) {
    for (unsigned int j=i+1; j<unit_size; j++) {
      Float dist2 = get_squared_distance(units[0][i], units[0][j]);
      r_dist.add_to_distribution(dist2, 2*form_factors[i]*form_factors[j]);
    }
    r_dist.add_to_distribution(0.0, square(form_factors[i]));
  }

  // distributions between units separated by distance i
  for(unsigned int in=1; in<number_of_distances; in++) {
    for (unsigned int i=0; i<unit_size; i++) {
      for (unsigned int j=0; j<unit_size; j++) {
        Float dist2 = get_squared_distance(units[0][i], units[in][j]);
        r_dist.add_to_distribution(dist2, 2*form_factors[i]*form_factors[j]);
      }
    }
  }
  r_dist.scale(n);

  // distribution between units separated by distance n/2
  RadialDistributionFunction r_dist2,r_dist2b;
  for (unsigned int i=0; i<unit_size; i++) {
    for (unsigned int j=0; j<unit_size; j++) {
      Float dist2 = get_squared_distance(units[0][i],
                                         units[number_of_distances][j]);
      r_dist2.add_to_distribution(dist2, 2*form_factors[i]*form_factors[j]);
    }
  }

  // if n is even, the scale is by n/2
  // if n is odd the scale is by n
  if(n & 1) r_dist2.scale(n); //odd
  else r_dist2.scale(n/2); //even
  r_dist2.add(r_dist);

  squared_distribution_2_profile(r_dist2,r_dist2b);
}

void Profile::calculate_profile_real(const Particles& particles1,
                                     const Particles& particles2,
                                     FormFactorType ff_type,
                                     bool variance, double variance_tau)
{
  IMP_LOG_TERSE( "start real profile calculation for "
          << particles1.size() << " + " << particles2.size()
          << " particles" << std::endl);
  RadialDistributionFunction r_dist; //fi(0) fj(0)
  RadialDistributionFunction r_dist2; //fi(0)^2 fj(0)^2

  // copy coordinates and form factors in advance, to avoid n^2 copy operations
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
      if (variance) r_dist2.add_to_distribution(dist, 2*prod*prod);
    }
  }
  squared_distribution_2_profile(r_dist, r_dist2, variance, variance_tau);
}

void Profile::distribution_2_profile(const RadialDistributionFunction& r_dist)
{
  init();
  // iterate over intensity profile
  for (unsigned int k = 0; k < profile_.size(); k++) {
    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.size(); r++) {
      Float dist = r_dist.index2dist(r);
      Float x = dist * profile_[k].q_;
      x = boost::math::sinc_pi(x);
      profile_[k].intensity_ += r_dist[r] * x;
    }
  }
}

void Profile::
squared_distribution_2_profile(const RadialDistributionFunction& r_dist,
        const RadialDistributionFunction& r_dist2,
        bool variance, double variance_tau)
{
  init();
  // precomputed sin(x)/x function
  static internal::SincFunction sf(sqrt(r_dist.get_max_distance())*get_max_q(),
                                   0.0001);

  // precompute square roots of distances
  std::vector<float> distances(r_dist.size(), 0.0);
  for (unsigned int r = 0; r < r_dist.size(); r++)
    if(r_dist[r] != 0.0)  distances[r] = sqrt(r_dist.index2dist(r));

  // iterate over intensity profile
  for (unsigned int k = 0; k < profile_.size(); k++) {
    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.size(); r++) {
      if(r_dist[r] != 0.0) {
        // x = sin(dq)/dq
        float dist = distances[r];
        float x = dist * profile_[k].q_;
        x = sf.sinc(x);
        // multiply by the value from distribution
        profile_[k].intensity_ += r_dist[r] * x;
      }
    }
    // this correction is required since we approximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    profile_[k].intensity_ *= std::exp(- modulation_function_parameter_
                                       * square(profile_[k].q_));
    if (variance)
        profile_[k].intensity_ *=
            std::exp(-0.5*square(variance_tau*profile_[k].q_));
  }

  if (!variance) return;

  /*for (double i=1; i<500; i+=.001){
      double Zval = internal::Z(.042,.0422,i);
      double Yval = internal::Y(.042,.0422,i,sf);
      std::cout << i << " " << Zval << " " << Yval << " " << Zval-Yval <<
      std::endl; }
  for (unsigned int r = 0; r < r_dist.size(); r++) {
      if (r_dist[r] != 0)
          std::cout << r << " " << r_dist[r] << " " << distances[r] <<
          std::endl; }*/
  // iterate over rows
  for (unsigned int i = 0; i < profile_.size(); i++) {
   // iterate over columns
   for (unsigned int j = i; j < profile_.size(); j++) {
     double q1 = profile_[i].q_;
     double q2 = profile_[j].q_;
     //double I1 = profile_[i].intensity_;
     //double I2 = profile_[j].intensity_;
     // iterate over radial distribution
     double contrib=0;
     if (q1*q2 != 0){ //else contrib is zero
      for (unsigned int r = 0; r < r_dist.size(); r++) {
        if(r_dist[r] == 0.0) continue;
        double dist = distances[r];
        if(dist == 0.0) continue;
        const double sqrt2 = 1.4142135623730950488;
        double a = q1*variance_tau/sqrt2;
        double b = q2*variance_tau/sqrt2;
        double c = dist/(variance_tau*sqrt2);
        //std::cout << "a " << a << " b " << b << " c " << c << std::endl;
        //exponent beats erf at high distances, so assume infs and nans mean 0
        double A(internal::A(a,b,c));
        //if (i==0 && j==0)
        //    std::cout << "a " << a << " b " << b
        //        << " c " << c << " A " << A << std::endl;
        //if (base::isinf(erfpart)) continue;
        //if (base::isnan(erfpart)) continue;
        contrib += A*r_dist2[r];
        //std::cout << a << " " << b << " " << c
        //    << " " << A  << " " << " " << dist
        //    << " " << r_dist2[r] << " " << contrib << std::endl;
        //std::cout << "contrib " << q1 << " " << q2 << " " << dist << " "
        //    << erfpart << " " << sincpart << " "
        //    << (erfpart-sincpart)*0.5*IMP::square(r_dist[r]) << std::endl;
      }
     }
    double var = contrib
                *std::exp(- modulation_function_parameter_*(q1*q1+q2*q2));
    variances_[i][j-i] = var;
   }
  }
}

void Profile::squared_distributions_2_partial_profiles(
                    const std::vector<RadialDistributionFunction>& r_dist)
{
  int r_size = r_dist.size();

  // init
  for(int i=0; i<r_size; i++) partial_profiles_[i].init();

  // precomputed sin(x)/x function
  static internal::SincFunction
    sf(sqrt(r_dist[0].get_max_distance())*get_max_q(), 0.0001);

  // precompute square roots of distances
  std::vector<float> distances(r_dist[0].size(), 0.0);
  for(unsigned int r = 0; r < r_dist[0].size(); r++)
    if(r_dist[0][r] > 0.0)  distances[r] = sqrt(r_dist[0].index2dist(r));

  // iterate over intensity profile
  for(unsigned int k = 0; k < partial_profiles_[0].size(); k++) {
    // iterate over radial distribution
    for(unsigned int r = 0; r < r_dist[0].size(); r++) {
      // x = sin(dq)/dq
      Float dist = distances[r];
      Float x = dist * partial_profiles_[0].profile_[k].q_;
      x = sf.sinc(x);
      // iterate over partial profiles
      if(r_dist[0][r] > 0.0) {
        for(int i=0; i<r_size; i++) {
          // multiply by the value from distribution
          partial_profiles_[i].profile_[k].intensity_ += r_dist[i][r] * x;
        }
      }
    }
    // this correction is required since we approximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    Float corr = std::exp(- modulation_function_parameter_ *
                          square(partial_profiles_[0].profile_[k].q_));
    for(int i=0; i<r_size; i++) {
      partial_profiles_[i].profile_[k].intensity_ *= corr;
    }
  }
}

void Profile::add(const Profile& other_profile, Float weight) {
  if(profile_.size() == 0 && other_profile.size() != 0) {
    min_q_ = other_profile.get_min_q();
    max_q_ = other_profile.get_max_q();
    delta_q_ = other_profile.get_delta_q();
    init();
  }
  // assumes same q values!!!
  for (unsigned int k = 0; k < profile_.size(); k++) {
    profile_[k].intensity_ += weight*other_profile.profile_[k].intensity_;
  }
}

void Profile::add_partial_profiles(const Profile& other_profile, Float weight) {
  if(other_profile.partial_profiles_.size() > 0 &&
     partial_profiles_.size() == 0) {
      partial_profiles_.insert(partial_profiles_.begin(),
                               other_profile.partial_profiles_.size(),
                               Profile(min_q_, max_q_, delta_q_));
  }
  if(partial_profiles_.size() != other_profile.partial_profiles_.size()) {
    IMP_WARN("Can't add different partial profile sizes "
             << partial_profiles_.size() << "-"
             << other_profile.partial_profiles_.size() << std::endl);
    return;
  }
  for(unsigned int i=0; i<partial_profiles_.size(); i++) {
    partial_profiles_[i].add(other_profile.partial_profiles_[i], weight);
  }
}

double Profile::radius_of_gyration_fixed_q(double end_q) const {
  IMP::algebra::Vector2Ds data; // x=q^2, y=logI(q)) z=error(q)/I(q)
  Floats errors;
  for(unsigned int i=0; i<profile_.size(); i++) {
    double q = profile_[i].q_;
    double Iq = profile_[i].intensity_;
    double err = profile_[i].error_/Iq;
    double logIq = log(Iq);
    if(q > end_q) break;
    algebra::Vector2D v(q*q,logIq);
    data.push_back(v);
    errors.push_back(err);
    //std::cout << q << " " << Iq << " " <<  q*q << " " << logIq << std::endl;
  }
  algebra::LinearFit2D lf(data, errors);
  double a = lf.get_a();
  //std::cerr  << "a = " << a <<  std::endl;
  if(a >=0) return 0.0;
  double rg = sqrt(-3*a);
  //std::cerr << "residuals = " << lf.get_fit_error() << std::endl;
  return rg;
}

double Profile::radius_of_gyration(double end_q_rg) const {
  double qlimit = min_q_ + delta_q_*5; // start after 5 points
  for(double q = qlimit; q<max_q_; q+=delta_q_) {
    double rg = radius_of_gyration_fixed_q(q);
    //std::cerr << "Rg = " << rg << " q*Rg = " << q*rg << std::endl;
    if(rg > 0.0) {
      if(q*rg < end_q_rg) qlimit = q;
      else break;
    }
  }
  double rg = radius_of_gyration_fixed_q(qlimit);
  //std::cerr << "Rg = " << rg  << std::endl;
  return rg;
}

void Profile::background_adjust(double start_q) {
  algebra::Vector2Ds data; // x=q^2, y=sum(q^2xI(q))
  double sum = 0.0;
  for(unsigned int i=0; i<profile_.size(); i++) {
    double q = profile_[i].q_;
    double Iq = profile_[i].intensity_;
    double q2xIq = q*q*Iq;
    sum+= q2xIq;
    //std::cout << q << " " << q2xIq << " " << sum << std::endl;
    if(q >= start_q) {
      algebra::Vector2D v(q*q, sum);
      data.push_back(v);
    }
  }

  algebra::ParabolicFit p(data);
  double P3 = p.get_a();
  double P2 = p.get_b();
  double P1 = p.get_c();
  double G1 = P2/P1;
  double G2 = 12.0*(P3/P1 - G1*G1/4.0);
  //std::cerr << "G1 = " << G1 << " G2 = " << G2 << std::endl;

  for(unsigned int i=0; i<profile_.size(); i++) {
    double q = profile_[i].q_;
    double q2 = q*q;
    double q4 = q2*q2;
    double Iq = profile_[i].intensity_;
    double Iq_new = Iq / (1.0 + q2*G1 + q4*(G1*G1/4.0 + G2/12.0));
    profile_[i].intensity_ = Iq_new;
    //profile.set_intensity(i, Iq_new);
    //std::cout << q << " " << Iq_new << " " << get_error(i)<< std::endl;
  }
}

void Profile::scale(Float c) {
  for (unsigned int k = 0; k < profile_.size(); k++) {
    profile_[k].intensity_ *= c;
  }
}

void Profile::offset(Float c) {
  for (unsigned int k = 0; k < profile_.size(); k++) {
    profile_[k].intensity_ -= c;
  }
}

void Profile::profile_2_distribution(RadialDistributionFunction& rd,
                                     Float max_distance) const {
  float scale = 1.0/(2*PI*PI);
  unsigned int distribution_size = rd.dist2index(max_distance) + 1;

  // offset profile so that minimal i(q) is zero
  float min_value = profile_[0].intensity_;
  for(unsigned int k = 0; k < profile_.size(); k++) {
    if(profile_[k].intensity_ < min_value)
      min_value = profile_[k].intensity_;
  }
  Profile p(min_q_, max_q_, delta_q_);
  p.init();
  for(unsigned int k = 0; k < profile_.size(); k++) {
    p.profile_[k].intensity_  = profile_[k].intensity_ - min_value;
  }

  // iterate over r
  for(unsigned int i = 0; i < distribution_size; i++) {
    Float r = rd.index2dist(i);
    Float sum = 0.0;
    // sum over q: SUM (I(q)*q*sin(qr))
    for(unsigned int k = 0; k < p.profile_.size(); k++) {
      sum += p.profile_[k].intensity_ *
        p.profile_[k].q_ * std::sin(p.profile_[k].q_*r);
    }
    rd.add_to_distribution(r, r*scale*sum);
  }
}

void Profile::calculate_profile_reciprocal(const Particles& particles,
                                           FormFactorType ff_type) {
  if(ff_type == CA_ATOMS) {
    IMP_WARN("Reciprocal space profile calculation is not suported for"
             << "residue level" << std::endl);
    return;
  }

  IMP_LOG_TERSE( "start reciprocal profile calculation for "
          << particles.size() << " particles" << std::endl);
  init();
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);

  // iterate over pairs of atoms
  // loop1
  for(unsigned int i = 0; i < coordinates.size(); i++) {
    const Floats& factors1 = ff_table_->get_form_factors(particles[i], ff_type);
    // loop2
    for(unsigned int j = i+1; j < coordinates.size(); j++) {
      const Floats& factors2=ff_table_->get_form_factors(particles[j], ff_type);
      Float dist = get_distance(coordinates[i], coordinates[j]);
      // loop 3
      // iterate over intensity profile
      for(unsigned int k = 0; k < profile_.size(); k++) {
        Float x = dist * profile_[k].q_;
        x = boost::math::sinc_pi(x);
        profile_[k].intensity_ += 2*x*factors1[k]*factors2[k];
      } // end of loop 3
    } // end of loop 2
    // add autocorrelation part
    for(unsigned int k = 0; k < profile_.size(); k++) {
      profile_[k].intensity_ += factors1[k]*factors1[k];
    }
  } // end of loop1
}

void Profile::calculate_profile_reciprocal_partial(const Particles& particles,
                                                   const Floats& surface,
                                                   FormFactorType ff_type) {
  if(ff_type == CA_ATOMS) {
    IMP_WARN("Reciprocal space profile calculation is not suported for"
             << "residue level" << std::endl);
    return;
  }

  IMP_LOG_TERSE( "start partial reciprocal profile calculation for "
          << particles.size() << " particles" << std::endl);

  init();
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);

  // allocate partial profiles
  int r_size = 3;
  if(surface.size() == particles.size()) r_size = 6;
  const Floats& water_ff = ff_table_->get_water_form_factors();
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           Profile(min_q_, max_q_, delta_q_));
  for(int i=0; i<r_size; i++) partial_profiles_[i].init();

  // iterate over pairs of atoms
  // loop1
  for(unsigned int i = 0; i < coordinates.size(); i++) {
    const Floats& vacuum_ff1 =
      ff_table_->get_vacuum_form_factors(particles[i], ff_type);
    const Floats& dummy_ff1 =
      ff_table_->get_dummy_form_factors(particles[i], ff_type);
    // loop2
    for(unsigned int j = i+1; j < coordinates.size(); j++) {
      const Floats& vacuum_ff2 =
        ff_table_->get_vacuum_form_factors(particles[j], ff_type);
      const Floats& dummy_ff2 =
        ff_table_->get_dummy_form_factors(particles[j], ff_type);
      Float dist = get_distance(coordinates[i], coordinates[j]);

      // loop 3
      // iterate over intensity profile
      for(unsigned int k = 0; k < profile_.size(); k++) {
        Float x = dist * profile_[k].q_;
        x = 2*boost::math::sinc_pi(x);
        // profile_[k].intensity_ += x*factors1[k]*factors2[k];
        partial_profiles_[0].profile_[k].intensity_ +=
          x*vacuum_ff1[k]*vacuum_ff2[k];
        partial_profiles_[1].profile_[k].intensity_ +=
          x*dummy_ff1[k]*dummy_ff2[k];
        partial_profiles_[2].profile_[k].intensity_ +=
          x * (vacuum_ff1[k]*dummy_ff2[k] + vacuum_ff2[k]*dummy_ff1[k]);

        if(r_size > 3) {
          partial_profiles_[3].profile_[k].intensity_ +=
            x * surface[i]*surface[j] * water_ff[k]*water_ff[k];
          partial_profiles_[4].profile_[k].intensity_ +=
            x * (vacuum_ff1[k] * surface[j] * water_ff[k] +
                 vacuum_ff2[k] * surface[i] * water_ff[k]);
          partial_profiles_[5].profile_[k].intensity_ +=
            x * (dummy_ff1[k] * surface[j] * water_ff[k] +
                 dummy_ff2[k] * surface[i] * water_ff[k]);
        }
      } // end of loop 3
    } // end of loop 2

    // add autocorrelation part
    for(unsigned int k = 0; k < profile_.size(); k++) {
      //profile_[k].intensity_ += factors1[k]*factors1[k];
      partial_profiles_[0].profile_[k].intensity_ +=
        vacuum_ff1[k]*vacuum_ff1[k];
      partial_profiles_[1].profile_[k].intensity_ +=
        dummy_ff1[k]*dummy_ff1[k];
      partial_profiles_[2].profile_[k].intensity_ +=
        2*vacuum_ff1[k]*dummy_ff1[k];

      if(r_size > 3) {
        partial_profiles_[3].profile_[k].intensity_ +=
          square(surface[i]*water_ff[k]);
        partial_profiles_[4].profile_[k].intensity_ +=
          2*vacuum_ff1[k]*surface[i]*water_ff[k];
        partial_profiles_[5].profile_[k].intensity_ +=
          2*dummy_ff1[k]*surface[i]*water_ff[k];
      }
    }
  } // end of loop1

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0, *this);
}

IMPSAXS_END_NAMESPACE

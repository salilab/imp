/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/SAXSProfile.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/algebra/utility.h>
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/utility.h>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <string>
#include <iomanip>

IMPSAXS_BEGIN_NAMESPACE

Float SAXSProfile::modulation_function_parameter_ = 0.23;

std::ostream & operator<<(std::ostream & s,
                          const SAXSProfile::IntensityEntry & e)
{
  return s << e.q_ << " " << e.intensity_ << " " << e.error_ << std::endl;
}

SAXSProfile::SAXSProfile(Float qmin, Float qmax, Float delta,
                         FormFactorTable * ff_table):
  min_q_(qmin), max_q_(qmax), delta_q_(delta), ff_table_(ff_table)
{
}

SAXSProfile::SAXSProfile(const String& file_name)
{
  read_SAXS_file(file_name);
}

void SAXSProfile::init()
{
  profile_.clear();
  unsigned int number_of_q_entries =
    algebra::round((max_q_ - min_q_) / delta_q_ ) + 1;

  for (unsigned int i=0; i<number_of_q_entries; i++) {
    IntensityEntry entry(min_q_ + i * delta_q_);
    profile_.push_back(entry);
  }
}

void SAXSProfile::read_SAXS_file(const String& file_name)
{
  std::ifstream in_file(file_name.c_str());

  if (!in_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
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
    boost::split(split_results, line, boost::is_any_of(" "),
                 boost::token_compress_on);
    if (split_results.size() != 2 && split_results.size() != 3)
      continue;                 // 3 values with error, 2 without
    entry.q_ = atof(split_results[0].c_str());
    entry.intensity_ = atof(split_results[1].c_str());
    if (split_results.size() == 3) {
      entry.error_ = atof(split_results[2].c_str());
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

  IMP_LOG(TERSE, "read_SAXS_file: " << file_name
          << " size= " << profile_.size() << " delta= " << delta_q_
          << " min_q= " << min_q_ << " max_q= " << max_q_ << std::endl);

  // saxs_read: No experimental error specified, error=0.3*I(q_max)
  if (!with_error) {
    add_errors();
    IMP_LOG(TERSE, "read_SAXS_file: No experimental error specified"
            << " -> error set to 0.3 I(q_max) = " << std::endl);
  }
}

void SAXSProfile::add_errors() {
  Float sig_exp = 0.3 * profile_[profile_.size() - 1].intensity_;
  for (unsigned int i=0; i<profile_.size(); i++)
    profile_[i].error_ = sig_exp;
}

bool SAXSProfile::is_uniform_sampling() const {
  if (profile_.size() <= 1) return false;

  Float curr_diff = profile_[1].q_ - profile_[0].q_;
  Float epsilon = curr_diff / 10;
  for (unsigned int i=2; i<profile_.size(); i++) {
    Float diff = profile_[i].q_ - profile_[i-1].q_;
    if(fabs(curr_diff - diff) > epsilon) return false;
  }
  return true;
}

void SAXSProfile::write_SAXS_file(const String& file_name)
{
  std::ofstream out_file(file_name.c_str());

  if (!out_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }

  // header line
  out_file.precision(15);
  out_file << "# SAXS profile: number of points = " << profile_.size()
           << ", q_min = " << min_q_ << ", q_max = " << max_q_;
  out_file << ", delta_q = " << delta_q_ << std::endl;
  out_file << "#       q            intensity         error" << std::endl;

  out_file.setf(std::ios::showpoint);
  out_file.precision(15);
  // Main data
  for (unsigned int i = 0; i < profile_.size(); i++) {
    out_file.setf(std::ios::left);
    out_file.width(20);
    out_file.fill('0');
    out_file << profile_[i].q_ << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file <<  profile_[i].intensity_ << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << profile_[i].error_ << std::endl;
  }
  out_file.close();
}

void SAXSProfile::calculate_profile_real(
                             const std::vector<Particle*>& particles)
{
  IMP_LOG(TERSE, "start real profile calculation for "
          << particles.size() << " particles" << std::endl);
  init();
  Float pr_resolution = 0.5;
  RadialDistributionFunction r_dist(pr_resolution, ff_table_);
  r_dist.calculate_distribution(particles);
  radial_distribution_2_profile(r_dist);
}

void SAXSProfile::calculate_profile_real(
                             const std::vector<Particle*>& particles,
                             unsigned int n)
{
  IMP_check(n > 1, "Attempting to use symmetric computation, symmetry order"
            << " should be > 1. Got: " << n, ValueException);
  IMP_LOG(TERSE, "start real profile calculation for " << particles.size()
          << " particles with symmetry = " << n << std::endl);
  init();
  // split units, only number_of_distances units is needed
  unsigned int number_of_distances = n/2;
  unsigned int unit_size = particles.size()/n;
  std::vector<Particles> units(number_of_distances+1, Particles(unit_size));
  for(unsigned int i=0; i<=number_of_distances; i++) {
    for(unsigned int j=0; j<unit_size; j++) {
      units[i][j] = particles[i*unit_size+j];
    }
  }

  Float pr_resolution = 0.5;
  RadialDistributionFunction r_dist(pr_resolution, ff_table_);
  // distribution within unit
  r_dist.calculate_distribution(units[0]);

  // distributions between units separated by distance i
  for(unsigned int i=1; i<number_of_distances; i++) {
    r_dist.calculate_distribution(units[0], units[i]);
  }
  r_dist.scale(n);

  // distribution between units separated by distance n/2
  RadialDistributionFunction r_dist2(pr_resolution, ff_table_);
  r_dist2.calculate_distribution(units[0], units[number_of_distances]);
  // if n is even, the scale is by n/2
  // if n is odd the scale is by n
  if(n & 1) r_dist2.scale(n); //odd
  else r_dist2.scale(n/2); //even
  r_dist2.add(r_dist);

  radial_distribution_2_profile(r_dist2);
}

void SAXSProfile::calculate_profile_real(
                      const std::vector<Particle *>& particles1,
                      const std::vector<Particle *>& particles2)
{
  IMP_LOG(TERSE, "start real profile calculation for "
          << particles1.size() << " + " << particles2.size()
          << " particles" << std::endl);
  init();
  Float pr_resolution = 0.5;
  RadialDistributionFunction r_dist(pr_resolution, ff_table_);
  r_dist.calculate_distribution(particles1, particles2);
  radial_distribution_2_profile(r_dist);
}

void SAXSProfile::
radial_distribution_2_profile(const RadialDistributionFunction & r_dist)
{
  // iterate over intensity profile (assumes initialized profile: q, I(q)=0)
  for (unsigned int k = 0; k < profile_.size(); k++) {

    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.distribution_.size(); r++) {
      Float dist = r_dist.index2dist(r);
      Float x = dist * profile_[k].q_;
      x = sinc(x);
      profile_[k].intensity_ += r_dist.distribution_[r] * x;
    }
    profile_[k].intensity_ *= std::exp(- modulation_function_parameter_
                                       * profile_[k].q_ * profile_[k].q_);
  }
}

void SAXSProfile::add(const SAXSProfile& other_profile) {
  // assumes same q values!!!
  for (unsigned int k = 0; k < profile_.size(); k++) {
    profile_[k].intensity_ += other_profile.profile_[k].intensity_;
  }
}

void SAXSProfile::scale(Float c) {
  for (unsigned int k = 0; k < profile_.size(); k++) {
    profile_[k].intensity_ *= c;
  }
}

IMPSAXS_END_NAMESPACE

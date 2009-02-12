/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/SAXSProfile.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/utility.h>
#include <IMP/algebra/utility.h>
#include <IMP/saxs/Distribution.h>
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
  min_q_(qmin), max_q_(qmax), delta_q_(delta),ff_table_(ff_table)
{
  pr_resolution_ = 0.5;

  init();
}


SAXSProfile::SAXSProfile(const String & file_name)
{
  pr_resolution_ = 0.5;

  read_SAXS_file(file_name);
}


void SAXSProfile::init()
{
  profile_.clear();
  unsigned int number_of_q_entries = algebra::round(
                                            (max_q_ - min_q_) / delta_q_ ) + 1;

  for (unsigned int i=0; i<number_of_q_entries; i++) {
    IntensityEntry entry(min_q_ + i * delta_q_);
    profile_.push_back(entry);
  }
}


void SAXSProfile::read_SAXS_file(const String & file_name)
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

  std::cerr << "Number of entries read " << profile_.size() << std::endl;
  in_file.close();

  // determine qmin, qmax and delta
  if (profile_.size() > 1) {
    min_q_ = profile_[0].q_;
    max_q_ = profile_[profile_.size() - 1].q_;

    //! To minimize rounding errors, by averaging differences of q
    std::vector<Float> diff;
    for (unsigned int i=1; i<profile_.size(); i++) {
      diff.push_back( profile_[i].q_ - profile_[i-1].q_ );
    }
    delta_q_ = 0.0;
    for (unsigned int i=0; i<diff.size(); i++) {
      delta_q_ += diff[i];
    }
    delta_q_ /= diff.size();
  }

  // saxs_read: No experimental error specified, error=0.3*I(q_max)
  if (!with_error) {
    Float sig_exp = 0.3 * profile_[profile_.size() - 1].intensity_;
    for (unsigned int i=0; i<profile_.size(); i++)
      profile_[i].error_ = sig_exp;
    std::cerr << "read_SAXS_file: No experimental error specified"
              << " -> error set to 0.3 I(q_max) = " << sig_exp << std::endl;
  }
}


void SAXSProfile::write_SAXS_file(const String & file_name)
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
  // Main data
  for (unsigned int i = 0; i < profile_.size(); i++) {
    out_file.setf(std::ios::left);
    out_file.width(20);
    out_file.fill('0');
    out_file << std::setprecision(15) << profile_[i].q_ << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << std::setprecision(15) << profile_[i].intensity_ << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << std::setprecision(15) << profile_[i].error_ << std::endl;
  }
  out_file.close();
}


void SAXSProfile::calculate_profile_real(
                             const std::vector<Particle*>& particles)
{
  init();

  std::cerr << "start real profile calculation for "
      << particles.size() << " particles" << std::endl;

  RadialDistributionFunction r_dist(pr_resolution_, ff_table_);
  r_dist.calculate_distribution(particles);
  radial_distribution_2_profile(r_dist);
  max_pr_distance_ = r_dist.get_max_pr_distance();
}


void SAXSProfile::calculate_profile_real(
                      const std::vector<Particle *>& particles1,
                      const std::vector<Particle *>& particles2)
{
  init();

  std::cerr << "start real profile calculation for "
      << particles1.size() << " + " << particles2.size()
      << " particles" << std::endl;

  RadialDistributionFunction r_dist(pr_resolution_, ff_table_);
  r_dist.calculate_distribution(particles1, particles2);
  radial_distribution_2_profile(r_dist);
  max_pr_distance_ = r_dist.get_max_pr_distance();
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

IMPSAXS_END_NAMESPACE

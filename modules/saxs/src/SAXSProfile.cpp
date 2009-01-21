/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/SAXSProfile.h>

#include <IMP/core/XYZDecorator.h>
#include <IMP/utility.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <string>

IMPSAXS_BEGIN_NAMESPACE

std::ostream & operator<<(std::ostream & s,
                          const SAXSProfile::IntensityEntry & e)
{
  return s << e.s_ << " " << e.intensity_ << " " << e.error_ << std::endl;
}

SAXSProfile::SAXSProfile(Float smin, Float smax, Float delta,
                         FormFactorTable * ff_table):
  min_s_(smin), max_s_(smax), delta_s_(delta),ff_table_(ff_table)
{
  init();
}

SAXSProfile::SAXSProfile(const String & file_name)
{
  read_SAXS_file(file_name);
}

void SAXSProfile::init()
{
  profile_.clear();
  int number_of_entries = (max_s_ - min_s_) / delta_s_ + 1;
  for (int i = 0; i < number_of_entries; i++) {
    IntensityEntry entry(min_s_ + i * delta_s_);
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
  // TODO: handle profiles with multiple comment lines
  // remove first comment line
  String line;
  getline(in_file, line);
  // TODO: handle profile reading without error column

  IntensityEntry entry;
  while (!in_file.eof()) {
    //while(in_file >> entry) {

    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (boost::all(line, boost::is_any_of(" \n\t")))
      continue;

    std::vector < std::string > split_results;
    boost::split(split_results, line, boost::is_any_of(" "));
    if (split_results.size() != 2 && split_results.size() != 3)
      continue;                 // 3 values with error, 2 without

    entry.s_ = atof(split_results[0].c_str());
    entry.intensity_ = atof(split_results[1].c_str());
    if (split_results.size() == 3)
      entry.error_ = atof(split_results[2].c_str());

    if (entry.error_ == 0.0) {
      std::cerr << "Zero intensity error! setting to 1" << std::endl;
      entry.error_ = 1.0;
    }
    profile_.push_back(entry);
  }

  std::cerr << "Number of entries read " << profile_.size() << std::endl;
  in_file.close();

  // detemine smin, smax and delta
  if (profile_.size() > 1) {
    min_s_ = profile_[0].s_;
    max_s_ = profile_[profile_.size() - 1].s_;
    delta_s_ = profile_[1].s_ - profile_[0].s_;
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
  out_file << "# SAXS profile: number of points = " << profile_.size()
      << " s_ min = " << min_s_ << " s_ max = " << max_s_
      << " delta = " << delta_s_ << std::endl;

  for (unsigned int i = 0; i < profile_.size(); i++) {
    out_file << profile_[i];
  }
  out_file.close();
}

void SAXSProfile::write_SAXS_fit_file(const String & file_name,
                                      const SAXSProfile & saxs_profile,
                                      const Float c) const
{
  std::ofstream out_file(file_name.c_str());

  if (!out_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }
  // header line
  out_file << "# SAXS profile: number of points = " << profile_.size()
           << " s_ min = " << min_s_ << " s_ max = " << max_s_
           << " delta = " << delta_s_ << std::endl;

  for (unsigned int i = 0; i < profile_.size(); i++) {
    out_file << profile_[i].s_ << "   " << c * profile_[i].intensity_ << "  "
        << saxs_profile.profile_[i].intensity_ << std::endl;
  }
  out_file.close();
}


void SAXSProfile::calculate_profile_real(
                             const std::vector<Particle*>& particles)
{
  init();

  std::cerr << "start real profile calculation for "
      << particles.size() << " particles" << std::endl;

  float delta = 0.5;            // add to parameter?
  RadialDistributionFunction r_dist(delta, ff_table_);
  r_dist.calculate_distribution(particles);
  radial_distribution_2_profile(r_dist);
}

void SAXSProfile::calculate_profile_real(
                      const std::vector<Particle *>& particles1,
                      const std::vector<Particle *>& particles2)
{
  std::cerr << "start real profile calculation for "
      << particles1.size() << " + " << particles2.size()
      << " particles" << std::endl;

  float delta = 0.5;            // TODO: add as parameter?
  RadialDistributionFunction r_dist(delta, ff_table_);
  r_dist.calculate_distribution(particles1, particles2);
  radial_distribution_2_profile(r_dist);
}

void SAXSProfile::
radial_distribution_2_profile(const RadialDistributionFunction & r_dist)
{
  init();
  float b = 0.23;               // TODO: add as parameter

  // iterate over intensity profile (assumes initialized profile: s, I(s)=0)
  for (unsigned int k = 0; k < profile_.size(); k++) {

    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.distribution_.size(); r++) {
      Float dist = r_dist.index2dist(r);
      Float x = dist * profile_[k].s_;
      x = sinc(x);
      profile_[k].intensity_ += r_dist.distribution_[r] * x;
    }
    profile_[k].intensity_ *= exp(-b * profile_[k].s_ * profile_[k].s_);
  }
}

Float SAXSProfile::compute_chi_score(const SAXSProfile & saxs_profile) const {

  // TODO: add some tests for the same sampling for both profiles

  // compute c value for fitting
  Float sum1(0.0), sum2(0.0);
  for (unsigned int k = 0; k < profile_.size(); k++) {

    // in the theoretical profile the error equals to 1
    Float square_error = square(saxs_profile.profile_[k].error_) *
      square(profile_[k].error_);

    sum1 += (profile_[k].intensity_ * saxs_profile.profile_[k].intensity_) /
      square_error;

    sum2 += square(profile_[k].intensity_) / square_error;
  }

  Float c = sum1 / sum2;
  std::cout << "c = " << c << std::endl;

  // compute chi
  Float chi = 0;

  for (unsigned int k = 0; k < profile_.size(); k++) {
    // in the theoretical profile the error equals to 1
    Float square_error = square(saxs_profile.profile_[k].error_) *
      square(profile_[k].error_);

    Float diff =
        saxs_profile.profile_[k].intensity_ - c * profile_[k].intensity_;

    chi += square(diff) / square_error;
  }

  chi /= profile_.size();

  // TODO: make this optional
  write_SAXS_fit_file("fitfile.dat", saxs_profile, c);

  return sqrt(chi);
}

IMPSAXS_END_NAMESPACE

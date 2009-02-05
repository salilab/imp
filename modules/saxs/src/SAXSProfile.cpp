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
  b_ = 0.23;
  pr_resolution_ = 0.5;
  max_pr_distance_ = 50.0;

  init();
}


SAXSProfile::SAXSProfile(const String & file_name)
{
  b_ = 0.23;
  pr_resolution_ = 0.5;
  max_pr_distance_ = 50.0;

  read_SAXS_file(file_name);
}


void SAXSProfile::init()
{
  profile_.clear();
  int number_of_entries = (int)((max_s_ - min_s_) / delta_s_ + 0.5) + 1;
  for (int i=0; i<number_of_entries; i++) {
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

/*
  getline(in_file, line);
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
*/
  int ncols=0;
  std::string line;
  profile_.clear();

  // It handles profiles with multiple comment lines
  while ( !in_file.eof() ) {
    getline(in_file, line);
    if (line[0] == '#' || line[0] == '\0')
      continue;

    Float s, intensity, error;
    ncols = sscanf(line.c_str(), "%lf %lf %lf", &s, &intensity, &error);
    IntensityEntry entry(s, intensity, error);
    profile_.push_back(entry);
  }
  std::cerr << "Number of entries read " << profile_.size() << std::endl;
  in_file.close();

  //! determine smin, smax and delta
  if (profile_.size() > 1) {
    min_s_ = profile_[0].s_;
    max_s_ = profile_[profile_.size() - 1].s_;
    // Corrected by SJ Kim (1/23/09)
    delta_s_ = (max_s_ - min_s_) / (profile_.size() - 1);
  }

  //! saxs_read: No experimental error specified (ncols < 3), error=0.3*I(s_max)
  if (ncols < 3) {
    Float sig_exp = 0.3 * profile_[profile_.size() - 1].intensity_;
    for (unsigned int i=0; i<profile_.size(); i++)
      profile_[i].error_ = sig_exp;
    std::cerr << "read_SAXS_file: No experimental error specified" << std::endl;
    std::cerr << "-> error set to 0.3 I(s_max) = " << sig_exp << std::endl;
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
           << ", s_min = " << min_s_ << ", s_max = " << max_s_;
  out_file << ", delta_s = " << delta_s_ << std::endl;
  out_file << "#       s            intensity         error" << std::endl;

  out_file.setf(std::ios::showpoint);
  // Main data
  for (unsigned int i = 0; i < profile_.size(); i++) {
    out_file.setf(std::ios::left);
    out_file.width(20);
    out_file.fill('0');
    out_file << std::setprecision(15) << profile_[i].s_ << " ";

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
  r_dist.set_max_pr_distance(max_pr_distance_);
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
  r_dist.set_max_pr_distance(max_pr_distance_);
  r_dist.calculate_distribution(particles1, particles2);
  radial_distribution_2_profile(r_dist);
  max_pr_distance_ = r_dist.get_max_pr_distance();
}


void SAXSProfile::
radial_distribution_2_profile(const RadialDistributionFunction & r_dist)
{
  init();

  // iterate over intensity profile (assumes initialized profile: s, I(s)=0)
  for (unsigned int k = 0; k < profile_.size(); k++) {

    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.distribution_.size(); r++) {
      Float dist = r_dist.index2dist(r);
      Float x = dist * profile_[k].s_;
      x = sinc(x);
      profile_[k].intensity_ += r_dist.distribution_[r] * x;
    }
    profile_[k].intensity_ *= std::exp(-b_ * profile_[k].s_ * profile_[k].s_);
  }
}

IMPSAXS_END_NAMESPACE

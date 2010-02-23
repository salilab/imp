/**
 *  \file Profile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/utility.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/constants.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <string>

IMPSAXS_BEGIN_NAMESPACE

const Float Profile::modulation_function_parameter_ = 0.23;

std::ostream & operator<<(std::ostream & s, const Profile::IntensityEntry & e)
{
  return s << e.q_ << " " << e.intensity_ << " " << e.error_ << std::endl;
}

Profile::Profile(FormFactorTable *ff_table,
                 Float qmin, Float qmax, Float delta):
  min_q_(qmin), max_q_(qmax), delta_q_(delta), ff_table_(ff_table)
{
}

Profile::Profile(const String& file_name)
{
  read_SAXS_file(file_name);
}

void Profile::init()
{
  profile_.clear();
  unsigned int number_of_q_entries = std::ceil((max_q_ - min_q_) / delta_q_ );

  for (unsigned int i=0; i<number_of_q_entries; i++) {
    IntensityEntry entry(min_q_ + i * delta_q_);
    profile_.push_back(entry);
  }
}

void Profile::read_SAXS_file(const String& file_name)
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
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() < 2) continue;
    entry.q_ = atof(split_results[0].c_str());
    entry.intensity_ = atof(split_results[1].c_str());
    if (split_results.size() >= 3) {
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

void Profile::add_errors() {
  if(profile_.size() <= 0) return;
  Float sig_exp = 0.3 * profile_[profile_.size() - 1].intensity_;
  for (unsigned int i=0; i<profile_.size(); i++)
    profile_[i].error_ = sig_exp;
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

void Profile::write_SAXS_file(const String& file_name)
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

void Profile::calculate_profile_real(const Particles& particles,
                                     bool autocorrelation)
{
  IMP_LOG(TERSE, "start real profile calculation for "
          << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist(ff_table_);
  r_dist.calculate_squared_distribution(particles, autocorrelation);
  squared_distribution_2_profile(r_dist);
}

void Profile::calculate_profile_real(const Particles& particles,
                                     unsigned int n)
{
  IMP_USAGE_CHECK(n > 1,
                  "Attempting to use symmetric computation, symmetry order"
            << " should be > 1. Got: " << n);
  IMP_LOG(TERSE, "start real profile calculation for " << particles.size()
          << " particles with symmetry = " << n << std::endl);
  // split units, only number_of_distances units is needed
  unsigned int number_of_distances = n/2;
  unsigned int unit_size = particles.size()/n;
  std::vector<Particles> units(number_of_distances+1, Particles(unit_size));
  for(unsigned int i=0; i<=number_of_distances; i++) {
    for(unsigned int j=0; j<unit_size; j++) {
      units[i].set(j, particles[i*unit_size+j]);
    }
  }

  RadialDistributionFunction r_dist(ff_table_);
  // distribution within unit
  r_dist.calculate_squared_distribution(units[0]);

  // distributions between units separated by distance i
  for(unsigned int i=1; i<number_of_distances; i++) {
    r_dist.calculate_squared_distribution(units[0], units[i]);
  }
  r_dist.scale(n);

  // distribution between units separated by distance n/2
  RadialDistributionFunction r_dist2(ff_table_);
  r_dist2.calculate_squared_distribution(units[0], units[number_of_distances]);
  // if n is even, the scale is by n/2
  // if n is odd the scale is by n
  if(n & 1) r_dist2.scale(n); //odd
  else r_dist2.scale(n/2); //even
  r_dist2.add(r_dist);

  squared_distribution_2_profile(r_dist2);
}

void Profile::calculate_profile_real(const Particles& particles1,
                                     const Particles& particles2)
{
  IMP_LOG(TERSE, "start real profile calculation for "
          << particles1.size() << " + " << particles2.size()
          << " particles" << std::endl);
  RadialDistributionFunction r_dist(ff_table_);
  r_dist.calculate_squared_distribution(particles1, particles2);
  squared_distribution_2_profile(r_dist);
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
      x = sinc(x);
      profile_[k].intensity_ += r_dist[r] * x;
    }
  }
}

void Profile::
squared_distribution_2_profile(const RadialDistributionFunction& r_dist)
{
  init();
  // iterate over intensity profile
  for (unsigned int k = 0; k < profile_.size(); k++) {
    // iterate over radial distribution
    for (unsigned int r = 0; r < r_dist.size(); r++) {
      Float dist = sqrt(r_dist.index2dist(r));
      Float x = dist * profile_[k].q_;
      x = sinc(x);
      profile_[k].intensity_ += r_dist[r] * x;
    }
    // this correction is required since we apporximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    profile_[k].intensity_ *= std::exp(- modulation_function_parameter_
                                       * square(profile_[k].q_));
  }
}

void Profile::add(const Profile& other_profile) {
  if(profile_.size() == 0 && other_profile.size() != 0) {
    min_q_ = other_profile.get_min_q();
    max_q_ = other_profile.get_max_q();
    delta_q_ = other_profile.get_delta_q();
    init();
  }
  // assumes same q values!!!
  for (unsigned int k = 0; k < profile_.size(); k++) {
    profile_[k].intensity_ += other_profile.profile_[k].intensity_;
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
  Profile p(ff_table_, min_q_, max_q_, delta_q_);
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
                                           bool autocorrelation) {
  IMP_LOG(TERSE, "start reciprocal profile calculation for "
          << particles.size() << " particles" << std::endl);
  init();
  std::vector<algebra::VectorD<3> > coordinates;
  copy_coordinates(particles, coordinates);

  // iterate over pairs of atoms
  // loop1
  for(unsigned int i = 0; i < coordinates.size(); i++) {
    const Floats& factors1 = ff_table_->get_form_factors(particles[i]);
    // loop2
    for(unsigned int j = i+1; j < coordinates.size(); j++) {
      const Floats& factors2 = ff_table_->get_form_factors(particles[j]);
      Float dist =get_distance(coordinates[i], coordinates[j]);
      // loop 3
      // iterate over intensity profile
      for(unsigned int k = 0; k < profile_.size(); k++) {
        Float x = dist * profile_[k].q_;
        x = sinc(x);
        profile_[k].intensity_ += 2*x*factors1[k]*factors2[k];
      } // end of loop 3
    } // end of loop 2
    // add autocorrelation part
    if(autocorrelation) {
      for(unsigned int k = 0; k < profile_.size(); k++) {
        profile_[k].intensity_ += factors1[k]*factors1[k];
      }
    }
  } // end of loop1
}

IMPSAXS_END_NAMESPACE

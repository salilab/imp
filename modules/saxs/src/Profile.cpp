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
#include <IMP/algebra/ParabolicFit.h>
#include <IMP/constants.h>

#include <boost/algorithm/string.hpp>
//#include <boost/timer.hpp>
#include <boost/random.hpp>

#include <fstream>
#include <string>

IMPSAXS_BEGIN_NAMESPACE

const Float Profile::modulation_function_parameter_ = 0.23;

std::ostream & operator<<(std::ostream & s, const Profile::IntensityEntry & e)
{
  return s << e.q_ << " " << e.intensity_ << " " << e.error_ << std::endl;
}

Profile::Profile(Float qmin, Float qmax, Float delta):
  min_q_(qmin), max_q_(qmax), delta_q_(delta), experimental_(false)
{
  ff_table_ = default_form_factor_table();
}

Profile::Profile(const String& file_name) : experimental_(true)
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
    IMP_THROW("Can't open file " << file_name,
              IOException);
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
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  // header line
  out_file << "# SAXS profile: number of points = " << profile_.size()
           << ", q_min = " << min_q_ << ", q_max = " << max_q_;
  out_file << ", delta_q = " << delta_q_ << std::endl;
  out_file << "#    q    intensity ";
  if(experimental_) out_file << "   error";
  out_file << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < profile_.size(); i++) {
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

void Profile::calculate_profile_real(const Particles& particles,
                                     bool autocorrelation)
{
  IMP_LOG(TERSE, "start real profile calculation for "
          << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist;

  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  Floats form_factors;
  copy_data(particles, ff_table_, coordinates, form_factors);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      r_dist.add_to_distribution(dist, 2.0* form_factors[i] * form_factors[j]);
    }
    // add autocorrelation part
    if(autocorrelation) r_dist.add_to_distribution(0.0,square(form_factors[i]));
  }
  squared_distribution_2_profile(r_dist);
}

void Profile::calculate_profile_partial(const Particles& particles,
                                        const Floats& surface,
                                        bool autocorrelation)
{
  IMP_LOG(TERSE, "start real partial profile calculation for "
          << particles.size() << " particles " <<  std::endl);

  //boost::timer my_timer;
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  copy_coordinates(particles, coordinates);
  Floats vacuum_ff(particles.size()), dummy_ff(particles.size()), water_ff;
  for (unsigned int i=0; i<particles.size(); i++) {
    vacuum_ff[i] = ff_table_->get_vacuum_form_factor(particles[i]);
    dummy_ff[i] = ff_table_->get_dummy_form_factor(particles[i]);
  }
  if(surface.size() == particles.size()) {
    water_ff.resize(particles.size());
    Float ff_water = ff_table_->get_water_form_factor();
    for (unsigned int i=0; i<particles.size(); i++) {
      water_ff[i] += surface[i]*ff_water;
    }
  }
  //my_timer.restart();

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
    if(autocorrelation) {
      r_dist[0].add_to_distribution(0.0, square(vacuum_ff[i]));
      r_dist[1].add_to_distribution(0.0, square(dummy_ff[i]));
      r_dist[2].add_to_distribution(0.0, 2*vacuum_ff[i] * dummy_ff[i]);
      if(r_size > 3) {
        r_dist[3].add_to_distribution(0.0, square(water_ff[i]));
        r_dist[4].add_to_distribution(0.0, 2*vacuum_ff[i] * water_ff[i]);
        r_dist[5].add_to_distribution(0.0, 2*water_ff[i] * dummy_ff[i]);
      }
    }
  }
  //std::cerr << "Distribution comp. time " << my_timer.elapsed() << std::endl;
  //my_timer.restart();

  // convert to reciprocal space
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           Profile(min_q_, max_q_, delta_q_));
  squared_distributions_2_partial_profiles(r_dist);
  //std::cerr << "Conversion to reciprocal time " << my_timer.elapsed()
  //<< std::endl;

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0, *this);
}

void Profile::calculate_profile_partial(const Particles& particles1,
                                        const Particles& particles2)
{
  IMP_LOG(TERSE, "start real partial profile calculation for "
          << particles1.size() << " particles + "
          << particles2.size() <<  std::endl);

  // store coordinates
  std::vector <algebra::Vector3D> coordinates1,coordinates2;
  copy_coordinates(particles1, coordinates1);
  copy_coordinates(particles2, coordinates2);
  // get form factors
  Floats vacuum_ff1(particles1.size()), dummy_ff1(particles1.size());
  for (unsigned int i=0; i<particles1.size(); i++) {
    vacuum_ff1[i] = ff_table_->get_vacuum_form_factor(particles1[i]);
    dummy_ff1[i] = ff_table_->get_dummy_form_factor(particles1[i]);
  }
  Floats vacuum_ff2(particles2.size()), dummy_ff2(particles2.size());
  for (unsigned int i=0; i<particles2.size(); i++) {
    vacuum_ff2[i] = ff_table_->get_vacuum_form_factor(particles2[i]);
    dummy_ff2[i] = ff_table_->get_dummy_form_factor(particles2[i]);
  }

  int r_size = 3;
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
    }
  }

  // convert to reciprocal space
  partial_profiles_.insert(partial_profiles_.begin(), r_size,
                           Profile(min_q_, max_q_, delta_q_));
  for(int i=0; i<r_size; i++) {
    partial_profiles_[i].squared_distribution_2_profile(r_dist[i]);
  }

  // compute default profile c1 = 1, c2 = 0
  sum_partial_profiles(1.0, 0.0, *this);
}



void Profile::sum_partial_profiles(Float c1, Float c2, Profile& out_profile) {
  out_profile.init();
  out_profile.add(partial_profiles_[0]);
  Profile p1, p2;
  p1.add(partial_profiles_[1]);
  p2.add(partial_profiles_[2]);
  p1.scale(c1*c1);
  p2.scale(-c1);
  out_profile.add(p1);
  out_profile.add(p2);

  if(partial_profiles_.size() > 3) {
    Profile p3, p4, p5;
    p3.add(partial_profiles_[3]);
    p4.add(partial_profiles_[4]);
    p5.add(partial_profiles_[5]);
    p3.scale(c2*c2);
    p4.scale(c2);
    p5.scale(-c1*c2);
    out_profile.add(p3);
    out_profile.add(p4);
    out_profile.add(p5);
  }
}

/*
void Profile::calculate_profile_real(const Particles& particles,
                                     unsigned int n)
{
  IMP_USAGE_CHECK(n > 1,
                  "Attempting to use symmetric computation, symmetry order"
            << " should be > 1. Got: " << n, ValueException);
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

  RadialDistributionFunction r_dist;
  // distribution within unit
  r_dist.calculate_squared_distribution(units[0]);

  // distributions between units separated by distance i
  for(unsigned int i=1; i<number_of_distances; i++) {
    r_dist.calculate_squared_distribution(units[0], units[i]);
  }
  r_dist.scale(n);

  // distribution between units separated by distance n/2
  RadialDistributionFunction r_dist2;
  r_dist2.calculate_squared_distribution(units[0], units[number_of_distances]);
  // if n is even, the scale is by n/2
  // if n is odd the scale is by n
  if(n & 1) r_dist2.scale(n); //odd
  else r_dist2.scale(n/2); //even
  r_dist2.add(r_dist);

  squared_distribution_2_profile(r_dist2);
}
*/

void Profile::calculate_profile_real(const Particles& particles1,
                                     const Particles& particles2)
{
  IMP_LOG(TERSE, "start real profile calculation for "
          << particles1.size() << " + " << particles2.size()
          << " particles" << std::endl);
  RadialDistributionFunction r_dist;
  //r_dist.calculate_squared_distribution(particles1, particles2);

  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates1, coordinates2;
  Floats form_factors1, form_factors2;
  copy_data(particles1, ff_table_, coordinates1, form_factors1);
  copy_data(particles2, ff_table_, coordinates2, form_factors2);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = get_squared_distance(coordinates1[i], coordinates2[j]);
      r_dist.add_to_distribution(dist, 2 * form_factors1[i] * form_factors2[j]);
    }
  }
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
      // x = sin(dq)/dq
      Float dist = sqrt(r_dist.index2dist(r));
      Float x = dist * profile_[k].q_;
      x = sinc(x);
      // multiply by the value from distribution
      profile_[k].intensity_ += r_dist[r] * x;
    }
    // this correction is required since we approximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    profile_[k].intensity_ *= std::exp(- modulation_function_parameter_
                                       * square(profile_[k].q_));
  }
}

void Profile::squared_distributions_2_partial_profiles(
                    const std::vector<RadialDistributionFunction>& r_dist)
{
  int r_size = r_dist.size();

  // init
  for(int i=0; i<r_size; i++) partial_profiles_[i].init();

  // iterate over intensity profile
  for(unsigned int k = 0; k < partial_profiles_[0].size(); k++) {
    // iterate over radial distribution
    for(unsigned int r = 0; r < r_dist[0].size(); r++) {
      // x = sin(dq)/dq
      Float dist = sqrt(r_dist[0].index2dist(r));
      Float x = dist * partial_profiles_[0].profile_[k].q_;
      x = sinc(x);
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

void Profile::add_partial_profiles(const Profile& other_profile) {
  if(partial_profiles_.size() != other_profile.partial_profiles_.size()) {
    IMP_WARN("Can't add different partial profile sizes "
             << partial_profiles_.size() << "-"
             << other_profile.partial_profiles_.size() << std::endl);
    return;
  }
  for(unsigned int i=0; i<partial_profiles_.size(); i++) {
    partial_profiles_[i].add(other_profile.partial_profiles_[i]);
  }
}

void Profile::background_adjust(double start_q) {
  std::vector<algebra::Vector2D> data; // x=q^2, y=sum(q^2xI(q))
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
                                           bool autocorrelation) {
  IMP_LOG(TERSE, "start reciprocal profile calculation for "
          << particles.size() << " particles" << std::endl);
  init();
  std::vector<algebra::Vector3D> coordinates;
  copy_coordinates(particles, coordinates);

  // iterate over pairs of atoms
  // loop1
  for(unsigned int i = 0; i < coordinates.size(); i++) {
    const Floats& factors1 = ff_table_->get_form_factors(particles[i]);
    // loop2
    for(unsigned int j = i+1; j < coordinates.size(); j++) {
      const Floats& factors2 = ff_table_->get_form_factors(particles[j]);
      Float dist = get_distance(coordinates[i], coordinates[j]);
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

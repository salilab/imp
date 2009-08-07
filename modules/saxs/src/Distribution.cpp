/*
 *  \file Distribution.cpp \brief computes
 *
 *  distribution classes implementation
 *
 *  Copyright 2009 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>

#include <boost/algorithm/string.hpp>

IMPSAXS_BEGIN_NAMESPACE

RadialDistributionFunction::
RadialDistributionFunction(FormFactorTable * ff_table, Float bin_size)
  : Distribution<Float>(bin_size), ff_table_(ff_table)
{
}

RadialDistributionFunction::
RadialDistributionFunction(const std::string& file_name)
: Distribution<Float>(pr_resolution)
{
  read_pr_file(file_name);
}


void RadialDistributionFunction::
calculate_distribution(const Particles& particles, bool autocorrelation)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  Floats form_factors;
  copy_data(particles, ff_table_, coordinates, form_factors);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = distance(coordinates[i], coordinates[j]);
      add_to_distribution(dist, 2.0 * form_factors[i] * form_factors[j]);
    }
    // add autocorrelation part
    if(autocorrelation)
      add_to_distribution(0.0, square(form_factors[i]));
  }
}

void RadialDistributionFunction::
calculate_squared_distribution(const Particles& particles, bool autocorrelation)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  Floats form_factors;
  copy_data(particles, ff_table_, coordinates, form_factors);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = squared_distance(coordinates[i], coordinates[j]);
      add_to_distribution(dist, 2.0 * form_factors[i] * form_factors[j]);
    }
    // add autocorrelation part
    if(autocorrelation) add_to_distribution(0.0, square(form_factors[i]));
  }
}

void RadialDistributionFunction::
calculate_distribution(const Particles& particles1, const Particles& particles2)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates1, coordinates2;
  Floats form_factors1, form_factors2;
  copy_data(particles1, ff_table_, coordinates1, form_factors1);
  copy_data(particles2, ff_table_, coordinates2, form_factors2);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = distance(coordinates1[i], coordinates2[j]);
      add_to_distribution(dist, 2 * form_factors1[i] * form_factors2[j]);
    }
  }
}

void RadialDistributionFunction::
calculate_squared_distribution(const Particles& particles1,
                               const Particles& particles2)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates1, coordinates2;
  Floats form_factors1, form_factors2;
  copy_data(particles1, ff_table_, coordinates1, form_factors1);
  copy_data(particles2, ff_table_, coordinates2, form_factors2);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = squared_distance(coordinates1[i], coordinates2[j]);
      add_to_distribution(dist, 2 * form_factors1[i] * form_factors2[j]);
    }
  }
}

void RadialDistributionFunction::scale(Float c)
{
  for (unsigned int i = 0; i < size(); i++) (*this)[i]*=c;
}

void RadialDistributionFunction::add(const RadialDistributionFunction& other_rd)
{
  for (unsigned int i = 0; i < other_rd.size(); i++) {
    add_to_distribution(other_rd.index2dist(i), other_rd[i]);
  }
}

Float RadialDistributionFunction::
R_factor_score(const RadialDistributionFunction& model_pr)
{
  Float sum1=0.0, sum2=0.0;
  unsigned int distribution_size = std::min(size(), model_pr.size());

  for(unsigned int i = 0; i < distribution_size; i++) {
    sum1 += std::abs((*this)[i] - model_pr[i]);
    sum2 += std::abs((*this)[i]);
  }
  return sum1/sum2;
}

Float RadialDistributionFunction::
chi_score(const RadialDistributionFunction& model_pr)
{
  Float chi_square = 0.0;
  unsigned int distribution_size = std::min(size(), model_pr.size());

  // compute chi
  for(unsigned int i = 0; i < distribution_size; i++) {
    chi_square += square(model_pr[i] - (*this)[i]);
  }
  chi_square /= distribution_size;
  return sqrt(chi_square);
}

void RadialDistributionFunction::write_fit_file(const std::string& file_name,
                                  const RadialDistributionFunction& model_pr)
{
  std::ofstream out_file(file_name.c_str());
  if(!out_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }

  unsigned int distribution_size = std::min(size(), model_pr.size());
  for(unsigned int i = 0; i < distribution_size; i++) {
    out_file << index2dist(i) << " " << (*this)[i]
                              << " " << model_pr[i] << std::endl;
  }
  out_file.close();
}

void RadialDistributionFunction::
show(std::ostream& out, std::string prefix) const
{
  for(unsigned int i = 0; i < size(); i++) {
    out << index2dist(i) << " " << (*this)[i] << std::endl;
  }
}

void RadialDistributionFunction::normalize()
{
  // calculate area
  Float sum = 0.0;
  for(unsigned int i=0; i<size(); i++) sum += (*this)[i];
  std::cerr << "sum = " << sum  << std::endl;

  // normalize
  for(unsigned int i=0; i <size(); i++) (*this)[i] /= sum;
}

void RadialDistributionFunction::read_pr_file(const std::string& file_name)
{
  const std::string TITLE_LINE = "Distance distribution";
  std::cerr << "start reading pr file " << file_name << std::endl;
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }

  double count = 0.0;
  std::string line;
  bool in_distribution = false;
  bool bin_size_set = false;
  while(!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    //std::cerr << line << std::endl;
    if(line.substr(0, TITLE_LINE.length()) == TITLE_LINE) {
      in_distribution = true;
      continue;
    }
    if(!in_distribution || line.length() == 0 ||
       line[0] == '\0' || !isdigit(line[0])) continue;
    // read distribution line
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() < 2) continue;
    Float r = atof(split_results[0].c_str());
    Float pr = atof(split_results[1].c_str());
    if(!bin_size_set && r > 0.0) {
      init(r);
      bin_size_set = true;
      std::cerr << "read_pr_file: bin_size set to " << r << std::endl;
    }
    add_to_distribution(r, pr);
    count += pr;
  }

  IMP_LOG(TERSE, "read_pr_file: " << file_name
          << " size=" << size() << " area="
          << count << std::endl);
}

DeltaDistributionFunction::
DeltaDistributionFunction(FormFactorTable* ff_table,
                          const Particles& particles,
                          Float max_distance, Float bin_size)
  : Distribution<algebra::Vector3D>(bin_size), ff_table_(ff_table)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  copy_data(particles, ff_table_, coordinates_, form_factors_);
  // compute max distance if not given
  max_distance_ = max_distance;
  if (max_distance_ <= 0.0) max_distance_ = compute_max_distance(particles);
}

void DeltaDistributionFunction::
calculate_derivative_distribution(Particle* particle)
{
  init();

  algebra::Vector3D particle_coordinate =
    core::XYZ::decorate_particle(particle).get_coordinates();
  Float particle_form_factor = ff_table_->get_form_factor(particle);
  for (unsigned int i=0; i<coordinates_.size(); i++) {
    Float dist = distance(coordinates_[i], particle_coordinate);
    algebra::Vector3D diff_vector = particle_coordinate - coordinates_[i];
    diff_vector *= particle_form_factor * form_factors_[i];
    add_to_distribution(dist, diff_vector);
  }
}

void DeltaDistributionFunction::
show(std::ostream& out, std::string prefix) const
{
  out << "DeltaDistributionFunction::show" << std::endl;
  for (unsigned int i = 0; i < size(); i++) {
    out << prefix << " dist " << index2dist(i) << " value " << (*this)[i]
        << std::endl;
  }
}

IMPSAXS_END_NAMESPACE

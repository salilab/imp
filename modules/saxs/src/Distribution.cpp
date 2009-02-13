/*
 *  \file Distribution.cpp \brief computes
 *
 *  distribution classes implementation
 *
 *  Copyright 2009 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/Distribution.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZDecorator.h>

IMPSAXS_BEGIN_NAMESPACE

Float compute_max_distance(const std::vector<Particle*>& particles) {
  Float max_dist2 = 0;
  std::vector < algebra::Vector3D > coordinates;
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates.push_back(core::XYZDecorator::cast(particles[i]).
                          get_coordinates());
  }
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist2 = squared_distance(coordinates[i], coordinates[j]);
      if(dist2 > max_dist2)
        max_dist2 = dist2;
    }
  }
  return sqrt(max_dist2);
}

RadialDistributionFunction::
RadialDistributionFunction(Float bin_size, FormFactorTable * ff_table)
  : Distribution<Float>(bin_size), ff_table_(ff_table)
{
}


void RadialDistributionFunction::
calculate_distribution(const std::vector<Particle*>& particles)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  Floats form_factors;
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates.push_back(core::XYZDecorator::cast(particles[i]).
                          get_coordinates());
    form_factors.push_back(ff_table_->get_form_factor(particles[i]));
  }

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = distance(coordinates[i], coordinates[j]);
      add_to_distribution(dist, 2.0 * form_factors[i] * form_factors[j]);
    }
    // add autocorrelation part
    add_to_distribution(0.0, square(form_factors[i]));
  }
}


void RadialDistributionFunction::
calculate_distribution(const std::vector<Particle*>&particles1,
                       const std::vector<Particle*>&particles2)
{
  // TODO: change to IMP macro
  std::cerr << "start distribution calculation for "
  << particles1.size() << " + " << particles2.size()
  << " particles" << std::endl;

  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates1, coordinates2;
  Floats form_factors1, form_factors2;
  for (unsigned int i = 0; i < particles1.size(); i++) {
    coordinates1.push_back(core::XYZDecorator::cast(particles1[i]).
                           get_coordinates());
    form_factors1.push_back(ff_table_->get_form_factor(particles1[i]));
  }
  for (unsigned int i = 0; i < particles2.size(); i++) {
    coordinates2.push_back(core::XYZDecorator::cast(particles2[i]).
                           get_coordinates());
    form_factors2.push_back(ff_table_->get_form_factor(particles2[i]));
  }

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = distance(coordinates1[i], coordinates2[j]);
      add_to_distribution(dist, form_factors1[i] * form_factors2[j]);
    }
  }
}


void RadialDistributionFunction::
show(std::ostream& out, std::string prefix) const
{
  for (unsigned int i = 0; i < distribution_.size(); i++) {
    out << prefix << " dist " << index2dist(i) << " " << distribution_[i]
    << std::endl;
  }
}


DeltaDistributionFunction::
DeltaDistributionFunction(Float bin_size,
                          FormFactorTable* ff_table,
                          const std::vector<Particle*>& particles)
  : Distribution<algebra::Vector3D>(bin_size), ff_table_(ff_table)
{
  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  coordinates_.resize(particles.size());
  form_factors_.resize(particles.size());
  for (unsigned int i=0; i<particles.size(); i++) {
    coordinates_[i] = core::XYZDecorator::cast(particles[i]).get_coordinates();
    form_factors_[i] = ff_table_->get_form_factor(particles[i]);
  }
  max_distance_ = compute_max_distance(particles);
}

void DeltaDistributionFunction::
calculate_derivative_distribution(Particle* particle)
{
  init();

  algebra::Vector3D particle_coordinate =
    core::XYZDecorator::cast(particle).get_coordinates();
  Float particle_form_factor = ff_table_->get_form_factor(particle);

  // delta_dist.distribution_ = sum_i [f_p(0) * f_i(0) * (x_p - x_i)]
  for (unsigned int i=0; i<coordinates_.size(); i++) {
    Float dist = distance(coordinates_[i], particle_coordinate);
    algebra::Vector3D diff_vector = particle_coordinate - coordinates_[i];
    diff_vector *= particle_form_factor * form_factors_[i];
    add_to_distribution(dist, diff_vector);
  }
}


void DeltaDistributionFunction::
show(std::ostream & out, std::string prefix) const
{
  for (unsigned int i = 0; i < distribution_.size(); i++) {
    out << prefix << " dist " << index2dist(i) << " (" << distribution_[i]
      //distribution_[i][0]
      //<< ", " << distribution_[i][1] << ", " << distribution_[i][2] << ")"
    << std::endl;
  }
}

IMPSAXS_END_NAMESPACE

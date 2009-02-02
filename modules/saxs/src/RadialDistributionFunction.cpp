/**
 * \file RadialDistributionFunction \brief computes radial distribution function
 * required for SAXS profile computation
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/RadialDistributionFunction.h>

#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZDecorator.h>

IMPSAXS_BEGIN_NAMESPACE

RadialDistributionFunction::RadialDistributionFunction(Float bin_size,
                                              FormFactorTable * ff_table):
bin_size_(bin_size), ff_table_(ff_table)
{
  distribution_.reserve(dist2index(50.0));      // start with ~50A
  //derivative_distribution_.reserve(dist2index(50.0));      // start with ~50A
}

void RadialDistributionFunction::add_to_distribution(Float dist, Float value)
{
  unsigned int index = dist2index(dist);
  if (index >= distribution_.size()) {
    distribution_.reserve(2 * index);   // to avoid many re-allocations
    distribution_.resize(index + 1, 0);
  }
  distribution_[index] += value;
}

/*
void RadialDistributionFunction::add_to_derivative_distribution
(Float dist, const algebra::Vector3D &value)
{
  unsigned int index = dist2index(dist);
  if (index >= derivative_distribution_.size()) {
    // to avoid many re-allocations
    derivative_distribution_.reserve(2 * index);
    derivative_distribution_.resize(index + 1, 0);
  }
  derivative_distribution_[index] += value;
}
*/


void RadialDistributionFunction::calculate_distribution(
                                const std::vector<Particle*>& particles)
{
  // TODO: add imp macro instead
  std::cerr << "start distribution calculation for "
      << particles.size() << " particles" << std::endl;

  // copy coordinates in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates.push_back(core::XYZDecorator::cast(particles[i]).
                          get_coordinates());
  }

  // iterate over pairs of atoms
  // loop1
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    Float factor1 = ff_table_->get_form_factor(particles[i]);
    // loop2
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float factor2 = ff_table_->get_form_factor(particles[j]);
      Float dist = distance(coordinates[i], coordinates[j]);
      add_to_distribution(dist, 2.0 * factor1 * factor2);
    }                           // end of loop 2

    // add autocorrelation part
    add_to_distribution(0.0, factor1 * factor1);
  }                             // end of loop1
}

/*
void RadialDistributionFunction::calculate_derivative_distribution(
                                const std::vector<Particle*>& particles)
{
  // TODO: add imp macro instead
  std::cerr << "start distribution calculation for "
  << particles.size() << " particles" << std::endl;

  // copy coordinates in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates;
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates.push_back(core::XYZDecorator::cast(particles[i]).
                          get_coordinates());
  }

  // iterate over pairs of atoms
  // loop1
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    Float factor1 = ff_table_->get_form_factor(particles[i]);
    // loop2
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float factor2 = ff_table_->get_form_factor(particles[j]);
      Float dist = distance(coordinates[i], coordinates[j]);
      algebra::Vector3D diff_vector = coordinates[i] - coordinates[j];
      diff_vector /= dist;
      //diff_vector
      add_to_derivative_distribution(dist, 2.0 * factor1 * factor2);
    }                           // end of loop 2

    // add autocorrelation part
    add_to_derivative_distribution(0.0, factor1 * factor1);
  }                             // end of loop1
}
 */


void RadialDistributionFunction::calculate_distribution(
                             const std::vector<Particle*>&particles1,
                             const std::vector<Particle*>&particles2)
{
  // TODO: change to IMP macro
  std::cerr << "start distribution calculation for "
      << particles1.size() << " + " << particles2.size()
      << " particles" << std::endl;

  // copy coordinates in advance, to avoid n^2 copy operations
  std::vector < algebra::Vector3D > coordinates1, coordinates2;
  for (unsigned int i = 0; i < particles1.size(); i++) {
    coordinates1.push_back(core::XYZDecorator::cast(particles1[i]).
                           get_coordinates());
  }
  for (unsigned int i = 0; i < particles2.size(); i++) {
    coordinates2.push_back(core::XYZDecorator::cast(particles2[i]).
                           get_coordinates());
  }

  // iterate over pairs of atoms
  // loop1
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    Float factor1 = ff_table_->get_form_factor(particles1[i]);
    // loop2
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float factor2 = ff_table_->get_form_factor(particles2[j]);
      Float dist = distance(coordinates1[i], coordinates2[j]);
      add_to_distribution(dist, factor1 * factor2);
    }                           // end of loop 2
  }                             // end of loop1
}

void RadialDistributionFunction::show(std::ostream & out, std::string prefix)
                                                                       const {
  for (unsigned int i = 0; i < distribution_.size(); i++) {
    out << prefix << " dist " << index2dist(i) << " " << distribution_[i]
        << std::endl;
  }
}


void RadialDistributionFunction::show(std::ostream & out)
const {
  for (unsigned int i = 0; i < distribution_.size(); i++) {
    //out << " dist " << index2dist(i) << " " << distribution_[i]
    //<< std::endl;
    printf("dist %g\t%.15f\n", index2dist(i), distribution_[i]);
  }
}

IMPSAXS_END_NAMESPACE

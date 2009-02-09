/*
 *  Distribution.cpp
 *  imp
 *
 *  Created by sjkim on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <IMP/saxs/Distribution.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZDecorator.h>

IMPSAXS_BEGIN_NAMESPACE

RadialDistributionFunction::
RadialDistributionFunction(Float bin_size, FormFactorTable * ff_table)
: Distribution<Float>(bin_size, ff_table)
{
}


void RadialDistributionFunction::
calculate_distribution(const std::vector<Particle*>& particles)
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

  // Pre-store zero_formfactor for all particles, for the faster calculation
  std::vector<Float> zero_formfactor(particles.size());
  for (unsigned int iatom=0; iatom<particles.size(); iatom++)
    zero_formfactor[iatom] = ff_table_->get_form_factor(particles[iatom]);

  // iterate over pairs of atoms
  // loop1
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    //Float factor1 = ff_table_->get_form_factor(particles[i]);
    Float factor1 = zero_formfactor[i];
    // loop2
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      //Float factor2 = ff_table_->get_form_factor(particles[j]);
      Float factor2 = zero_formfactor[j];
      Float dist = distance(coordinates[i], coordinates[j]);
      add_to_distribution(dist, 2.0 * factor1 * factor2);
    }                           // end of loop 2

    // add autocorrelation part
    add_to_distribution(0.0, factor1 * factor1);
  }                             // end of loop1
}


void RadialDistributionFunction::
calculate_distribution(const std::vector<Particle*>&particles1,
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


void RadialDistributionFunction::
show(std::ostream & out, std::string prefix) const
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
: Distribution<algebra::Vector3D>(bin_size, ff_table)
{
  //!----- copy coordinates in advance, to avoid n^2 copy operations
  coordinates_.resize(particles.size());
  for (unsigned int i=0; i<particles.size(); i++) {
    coordinates_[i] = core::XYZDecorator::cast(particles[i]).get_coordinates();
  }

  //!----- Pre-store zero_formfactor for all particles, for faster calculation
  zero_formfactor_.resize(particles.size());
  for (unsigned int i=0; i<particles.size(); i++)
    zero_formfactor_[i] = ff_table_->get_form_factor(particles[i]);
}


void DeltaDistributionFunction::
calculate_derivative_distribution(unsigned int i)
{
  distribution_.clear();
  distribution_.reserve(dist2index(max_pr_distance_));
  Float zero_formfactor_i = zero_formfactor_[i];

  //!----- delta_dist.distribution_ = sum_j [f_i(0) * f_j(0) * (x_i - x_j)]
  for (unsigned int j=0; j<coordinates_.size(); j++) {
    if (i==j) continue;

    Float dist = distance(coordinates_[i], coordinates_[j]);
    algebra::Vector3D diff_vector = coordinates_[i] - coordinates_[j];
    diff_vector *= zero_formfactor_i * zero_formfactor_[j];
    add_to_distribution(dist, diff_vector);
  }
}


void DeltaDistributionFunction::
show(std::ostream & out, std::string prefix) const
{
  for (unsigned int i = 0; i < distribution_.size(); i++) {
    out << prefix << " dist " << index2dist(i) << " (" << distribution_[i][0]
    << ", " << distribution_[i][1] << ", " << distribution_[i][2] << ")"
    << std::endl;
  }
}

IMPSAXS_END_NAMESPACE

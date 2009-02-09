/**
 * \file DeltaDistributionFunction \brief computes delta distribution function
 * required for calculation of SAXS Score derivatives
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/saxs/DeltaDistributionFunction.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZDecorator.h>

IMPSAXS_BEGIN_NAMESPACE


DeltaDistributionFunction::DeltaDistributionFunction(Float bin_size,
                                                     FormFactorTable* ff_table,
                                       const std::vector<Particle*>& particles):
bin_size_(bin_size), ff_table_(ff_table)
{
  max_pr_distance_ = 50.0;      // start with ~50A (by default)
  one_over_bin_size_ = 1.0 / bin_size_;     // for faster calculation
  distribution_.reserve(dist2index(max_pr_distance_));

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


void DeltaDistributionFunction::add_to_derivative_distribution
                                    (Float dist, const algebra::Vector3D &value)
{
  unsigned int index = dist2index(dist);
  if (index >= distribution_.size()) {
    //!----- to avoid many re-allocations
    distribution_.reserve(2 * index);
    distribution_.resize(index + 1, algebra::Vector3D(0.0,0.0,0.0));
    max_pr_distance_ = (index + 1) * bin_size_;
  }
  distribution_[index] += value;
}


void
DeltaDistributionFunction::calculate_derivative_distribution(unsigned int i)
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
    add_to_derivative_distribution(dist, diff_vector);
  }
}


void DeltaDistributionFunction::show(std::ostream & out, std::string prefix)
const {
  for (unsigned int i = 0; i < distribution_.size(); i++) {
    out << prefix << " dist " << index2dist(i) << " (" << distribution_[i][0]
        << ", " << distribution_[i][1] << ", " << distribution_[i][2] << ")"
        << std::endl;
  }
}


IMPSAXS_END_NAMESPACE

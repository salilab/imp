/**
 * \file Distribution.h \brief computes distribution functions
 *
 * Distribution - base distance distribution class
 * RadialDistributionFunction required for calculation of SAXS profile
 * DeltaDistributionFunction requires for chi-square derivatives
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_DISTRIBUTION_H
#define IMPSAXS_DISTRIBUTION_H

#include "config.h"
#include "FormFactorTable.h"
#include "SAXSProfile.h"
#include "SAXSScore.h"
#include <IMP/algebra/utility.h>
#include <IMP/Particle.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

/**
base class for distribution classes
*/
template<class ValueT>
class Distribution {
public:
  //! Constructor
  Distribution(Float bin_size) {
    bin_size_ = bin_size;
    one_over_bin_size_ = 1.0 / bin_size_;     // for faster calculation
    max_distance_ = 50.0;      // start with ~50A (by default)
    distribution_.reserve(dist2index(max_distance_) + 1);
  }

  //! get distribution as array of ValueT
  std::vector< ValueT > get_distribution() const { return distribution_; }

  //! returns distribution vector size
  unsigned int size() const { return distribution_.size(); }

  //! returns maximal distance value of distribution
  Float get_max_distance() { return max_distance_; }

  //! returns bin size
  Float get_bin_size_() { return bin_size_; }

protected:
  unsigned int dist2index(Float dist) const {
    return algebra::round( dist * one_over_bin_size_ );
  }
  Float index2dist(unsigned int index) const { return index * bin_size_; }

protected:
  std::vector< ValueT > distribution_;
  Float bin_size_, one_over_bin_size_; // resolution of discretization
  Float max_distance_;  // parameter for maximum r value for p(r) function
};

/**
 Radial Distribution class for calculating SAXS Profile
 this is distance distribution multiplied by form factors of atoms
*/
class IMPSAXSEXPORT RadialDistributionFunction : public Distribution<Float> {

public:
  //! Constructor
  RadialDistributionFunction(Float bin_size, FormFactorTable* ff_table);

  friend class SAXSProfile;

  //! computes radial distribution function for a set of particles
  void calculate_distribution(const std::vector<Particle*>& particles);

  //! computes distribution contribution from inter-molecular
  //! interactions between the particles
  void calculate_distribution(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  //! scale distribution by a constant
  void scale(Float c);

  //! add another distribution
  void add(const RadialDistributionFunction& other_rdf);

  //! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

 private:
  void add_to_distribution(Float dist, Float value) {
    unsigned int index = dist2index(dist);
    if (index >= distribution_.size()) {
      if(distribution_.capacity() <= index)
        distribution_.reserve(2 * index);   // to avoid many re-allocations
      distribution_.resize(index + 1, 0);
      max_distance_ = index2dist(index + 1);
    }
    distribution_[index] += value;
  }

 protected:
  FormFactorTable* ff_table_; // pointer to form factors table
};


/**
Delta Distribution class for calculating the derivatives of SAXS Score
this distribution is:
sum_i [f_p(0) * f_i(0) * (x_p - x_i)]
sum_i [f_p(0) * f_i(0) * (y_p - y_i)]
sum_i [f_p(0) * f_i(0) * (z_p - z_i)]
*/
class IMPSAXSEXPORT
DeltaDistributionFunction : public Distribution<algebra::Vector3D> {
public:
  //! Constructor
  DeltaDistributionFunction(Float bin_size, FormFactorTable* ff_table,
                            const std::vector<Particle*>& particles);

  friend class SAXSScore;

  //! calculates distribution for an atom defined by particle
  void calculate_derivative_distribution(Particle* particle);

  //! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

 private:
  void add_to_distribution(Float dist, const algebra::Vector3D& value) {
    unsigned int index = dist2index(dist);
    if (index >= distribution_.size()) {
      std::cerr << "DeltaDistributionFunction::add_to_distribution"
                << " - distance out of range " << std::endl;
      return;
    }
    distribution_[index] += value;
  }

  void init() {
    distribution_.clear();
    distribution_.insert(distribution_.begin(),
                         dist2index(max_distance_) + 1,
                         algebra::Vector3D(0.0, 0.0, 0.0));
  }

 protected:
  FormFactorTable* ff_table_; // pointer to form factors table
  std::vector<algebra::Vector3D> coordinates_;
  Floats form_factors_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DISTRIBUTION_H */

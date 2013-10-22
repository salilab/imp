/**
 * \file IMP/saxs/Distribution.h \brief computes distribution functions
 *
 * Distribution - base distance distribution class
 * RadialDistributionFunction required for calculation of SAXS profile
 * DeltaDistributionFunction requires for chi-square derivatives
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_DISTRIBUTION_H
#define IMPSAXS_DISTRIBUTION_H

#include <IMP/saxs/saxs_config.h>
#include "Profile.h"
#include <IMP/kernel/Particle.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

namespace { // anonymous
  static const Float pr_resolution = 0.5;
}

/**
base class for distribution classes
*/
template<class ValueT>
class Distribution : public std::vector< ValueT > {
public:
  //! Constructor
  Distribution(Float bin_size = pr_resolution) { init(bin_size); }

  //! returns maximal distance value of distribution
  Float get_max_distance() const { return max_distance_; }

  //! returns bin size
  Float get_bin_size() const { return bin_size_; }

protected:
  void init(Float bin_size) {
    //  clear();
    bin_size_ = bin_size;
    one_over_bin_size_ = 1.0/bin_size_;     // for faster calculation
    max_distance_ = 50.0;      // start with ~50A (by default)
    std::vector< ValueT >::reserve(dist2index(max_distance_) + 1);
  }
  unsigned int dist2index(Float dist) const {
    return algebra::get_rounded( dist * one_over_bin_size_ );
  }
  Float index2dist(unsigned int index) const { return index * bin_size_; }
protected:
  Float bin_size_, one_over_bin_size_; // resolution of discretization
  Float max_distance_;  // parameter for maximum r value for p(r) function
};

#ifdef SWIG
%template(FloatDistribution) Distribution<Float>;
%template(VectorDistribution) Distribution<algebra::Vector3D>;
#endif

/**
 Radial Distribution class for calculating SAXS Profile
 this is distance distribution multiplied by form factors of atoms
*/
class IMPSAXSEXPORT RadialDistributionFunction : public Distribution<Float> {

public:
  //! Constructor (default)
  RadialDistributionFunction(Float bin_size = pr_resolution);

  //! Constructor from gnom file
  RadialDistributionFunction(const std::string& file_name);

  friend class Profile;

  //! scale distribution by a constant
  void scale(Float c);

  //! add another distribution
  void add(const RadialDistributionFunction& model_pr);

  //! print tables
  void show(std::ostream &out=std::cout) const;

  //! analogy crystallographic R-factor score
  Float R_factor_score(const RadialDistributionFunction& model_pr,
                       const std::string& file_name = "") const;

  // //! analogy to chi score \untested{chi_score}
  // Float chi_score(const RadialDistributionFunction& model_pr) const;

  //! fit the distributions by scaling according to maximum
  Float fit(const RadialDistributionFunction& model_pr,
            const std::string& file_name = "") const;

  //! normalize to area = 1.0
  void normalize();

 private:

  void add_to_distribution(Float dist, Float value) {
    unsigned int index = dist2index(dist);
    if(index >= size()) {
      if(capacity() <= index)
        reserve(2 * index);   // to avoid many re-allocations
      resize(index + 1, 0);
      max_distance_ = index2dist(index + 1);
    }
    (*this)[index] += value;
  }

  //! read gnom file
  void read_pr_file(const std::string& file_name);

  //! write fit file for the two distributions
  void write_fit_file(const RadialDistributionFunction& model_pr,
                      Float c = 1.0, const std::string& file_name = "") const;

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
  DeltaDistributionFunction(const kernel::Particles& particles,
                            Float max_distance = 0.0,
                            Float bin_size = pr_resolution);

  //! calculates distribution for an atom defined by particle
  void calculate_derivative_distribution(kernel::Particle* particle);

  //! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

 private:
  void add_to_distribution(Float dist, const algebra::Vector3D& value) {
    unsigned int index = dist2index(dist);
    if(index >= size()) {
      if(capacity() <= index)
        reserve(2 * index);   // to avoid many re-allocations
      resize(index + 1, algebra::Vector3D(0.0, 0.0, 0.0));
      max_distance_ = index2dist(index + 1);
    }
    (*this)[index] += value;
  }

  void init() {
    clear();
    insert(begin(), dist2index(max_distance_) + 1,
           algebra::Vector3D(0.0, 0.0, 0.0));
  }

 protected:
  std::vector<algebra::Vector3D> coordinates_;
  Floats form_factors_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DISTRIBUTION_H */

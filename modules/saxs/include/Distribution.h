/**
 * \file IMP/saxs/Distribution.h \brief computes distribution functions
 *
 * Distribution - base distance distribution class
 * RadialDistributionFunction required for calculation of SAXS profile
 * DeltaDistributionFunction requires for chi-square derivatives
 *
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_DISTRIBUTION_H
#define IMPSAXS_DISTRIBUTION_H

#include <IMP/saxs/saxs_config.h>
#include "Profile.h"
#include <IMP/Particle.h>

#include <iostream>

IMPSAXS_BEGIN_NAMESPACE

namespace {  // anonymous
static const double pr_resolution = 0.5;
}

/**
base class for distribution classes
*/
template <class ValueT>
class Distribution : public std::vector<ValueT> {
 public:
  //! Constructor
  Distribution(double bin_size = pr_resolution) { init(bin_size); }

  //! returns maximal distance value of distribution
  double get_max_distance() const { return max_distance_; }

  //! returns bin size
  double get_bin_size() const { return bin_size_; }

  unsigned int get_index_from_distance(double dist) const {
    return algebra::get_rounded(dist * one_over_bin_size_);
  }
  double get_distance_from_index(unsigned int index) const {
    return index * bin_size_;
  }

 protected:
  void init(double bin_size) {
    //  clear();
    bin_size_ = bin_size;
    one_over_bin_size_ = 1.0 / bin_size_;  // for faster calculation
    max_distance_ = 50.0;                  // start with ~50A (by default)
    std::vector<ValueT>::reserve(get_index_from_distance(max_distance_) + 1);
  }

 protected:
  double bin_size_, one_over_bin_size_;  // resolution of discretization
  double max_distance_;  // parameter for maximum r value for p(r) function
};

#ifdef SWIG
%template(FloatDistribution) Distribution<double>;
%template(VectorDistribution) Distribution<algebra::Vector3D>;
#endif

/**
 Radial Distribution class for calculating SAXS Profile
 this is distance distribution multiplied by form factors of atoms
*/
class IMPSAXSEXPORT RadialDistributionFunction : public Distribution<double> {
  mutable std::vector<double> sqrt_distances_;

 public:
  //! Constructor (default)
  RadialDistributionFunction(double bin_size = pr_resolution);

  //! Constructor from gnom file
  RadialDistributionFunction(const std::string& file_name);

  //! Get square root of distance for each distribution point
  /** This is cached, and points to internal storage, so should not be used
      after the distribution is modified or destroyed. */
  const std::vector<double> &get_square_root_distances() const {
    size_t sz = size();
    if (sz > sqrt_distances_.size()) {
      sqrt_distances_.reserve(sz);
      for (unsigned int r = sqrt_distances_.size(); r < sz; ++r) {
        sqrt_distances_.push_back(sqrt(get_distance_from_index(r)));
      }
    }
    return sqrt_distances_;
  }

  //! scale distribution by a constant
  void scale(double c);

  //! add another distribution
  void add(const RadialDistributionFunction& model_pr);

  //! print tables
  void show(std::ostream& out = std::cout) const;

  //! analogy crystallographic R-factor score
  double R_factor_score(const RadialDistributionFunction& model_pr,
                        const std::string& file_name = "") const;

  // //! analogy to chi score \untested{chi_score}
  // double chi_score(const RadialDistributionFunction& model_pr) const;

  //! fit the distributions by scaling according to maximum
  double fit(const RadialDistributionFunction& model_pr,
             const std::string& file_name = "") const;

  //! normalize to area = 1.0
  void normalize();

  void add_to_distribution(double dist, double value) {
    unsigned int index = get_index_from_distance(dist);
    if (index >= size()) {
      if (capacity() <= index)
        reserve(2 * index);  // to avoid many re-allocations
      resize(index + 1, 0);
      max_distance_ = get_distance_from_index(index + 1);
    }
    (*this)[index] += value;
  }

 private:
  //! read gnom file
  void read_pr_file(const std::string& file_name);

  //! write fit file for the two distributions
  void write_fit_file(const RadialDistributionFunction& model_pr, double c = 1.0,
                      const std::string& file_name = "") const;
};

/**
Delta Distribution class for calculating the derivatives of SAXS Score
this distribution is:
sum_i [f_p(0) * f_i(0) * (x_p - x_i)]
sum_i [f_p(0) * f_i(0) * (y_p - y_i)]
sum_i [f_p(0) * f_i(0) * (z_p - z_i)]
*/
class IMPSAXSEXPORT DeltaDistributionFunction
    : public Distribution<algebra::Vector3D> {
 public:
  //! Constructor
  DeltaDistributionFunction(const Particles& particles,
                            double max_distance = 0.0,
                            double bin_size = pr_resolution);

  //! calculates distribution for an atom defined by particle
  void calculate_derivative_distribution(Particle* particle);

  //! print tables
  void show(std::ostream& out = std::cout, std::string prefix = "") const;

 private:
  void add_to_distribution(double dist, const algebra::Vector3D& value) {
    unsigned int index = get_index_from_distance(dist);
    if (index >= size()) {
      if (capacity() <= index)
        reserve(2 * index);  // to avoid many re-allocations
      resize(index + 1, algebra::Vector3D(0.0, 0.0, 0.0));
      max_distance_ = get_distance_from_index(index + 1);
    }
    (*this)[index] += value;
  }

  void init() {
    clear();
    insert(begin(), get_index_from_distance(max_distance_) + 1,
           algebra::Vector3D(0.0, 0.0, 0.0));
  }

 protected:
  std::vector<algebra::Vector3D> coordinates_;
  Vector<double> form_factors_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DISTRIBUTION_H */

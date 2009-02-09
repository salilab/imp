/**
 * \file Distribution.h \brief computes
 *
 * base distribution class
 * required for calculation of SAXS profile and SAXS chi-square derivates
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

/*
!----------------------------------------------------------------------
! base class for distribution classes
!----------------------------------------------------------------------
*/
template<class ValueT>
class Distribution {
public:
  Distribution(Float bin_size, FormFactorTable* ff_table) {
    bin_size_ = bin_size;   ff_table_ = ff_table;
    one_over_bin_size_ = 1.0 / bin_size_;     // for faster calculation
    max_pr_distance_ = 50.0;      // start with ~50A (by default)
    distribution_.reserve(dist2index(max_pr_distance_));
  }

  void add_to_distribution(Float dist, const ValueT& value) {
    unsigned int index = dist2index(dist);
    if (index >= distribution_.size()) {
      distribution_.reserve(2 * index);   //!----- to avoid many re-allocations
      //! Generalized initialization of a ValueT variable
      ValueT valueT_zero;   valueT_zero *= 0.0;
      distribution_.resize(index + 1, valueT_zero);
      max_pr_distance_ = (index + 1) * bin_size_;
    }
    distribution_[index] += value;
  }

  //! get distribution as array of ValueT
  std::vector< ValueT > get_distribution() const { return distribution_; }
  unsigned int size() const { return distribution_.size(); }
  Float get_max_pr_distance(void) { return max_pr_distance_; }
  Float get_bin_size_(void) { return bin_size_; }

protected:
  unsigned int dist2index(Float dist) const {
    return algebra::round( dist * one_over_bin_size_ );
  }
  Float index2dist(unsigned int index) const { return index * bin_size_; }

protected:
  std::vector< ValueT > distribution_;
  FormFactorTable* ff_table_; // pointer to form factors table
  Float bin_size_, one_over_bin_size_; // resolution of discretization
  Float max_pr_distance_;  // paramter for maximum r value for p(r) function
};


/*
 !----------------------------------------------------------------------
 ! Radial Distribution class for calculating SAXS Profile
 !----------------------------------------------------------------------
 */
class IMPSAXSEXPORT RadialDistributionFunction : public Distribution<Float> {

public:
  RadialDistributionFunction(Float bin_size, FormFactorTable* ff_table);

  friend class SAXSProfile;

  void calculate_distribution(const std::vector<Particle*>& particles);

  //! computes distribution contribution from inter-molecular
  // interactions between the particles
  void calculate_distribution(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  // ! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;
};


/*
!----------------------------------------------------------------------
! Delta Distribution class for calculating the derivatives of SAXS Score
!----------------------------------------------------------------------
*/
class IMPSAXSEXPORT
DeltaDistributionFunction : public Distribution<algebra::Vector3D> {
public:
  DeltaDistributionFunction(Float bin_size, FormFactorTable* ff_table,
                            const std::vector<Particle*>& particles);

  friend class SAXSScore;

  void calculate_derivative_distribution(unsigned int iatom);
  void calculate_derivative_distribution(
                                      const std::vector<Particle*>& particles1,
                                      const std::vector<Particle*>& particles2);

  // ! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

private:
  std::vector< algebra::Vector3D > coordinates_;
  std::vector<Float> zero_formfactor_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DISTRIBUTION_H */

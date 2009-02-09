/**
 * \file DeltaDistributionFunction.h \brief computes
 *
 * delta distribution function
 * required for calculation of SAXS Score derivatives
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_DELTA_DISTRIBUTION_FUNCTION_H
#define IMPSAXS_DELTA_DISTRIBUTION_FUNCTION_H

#include "config.h"
#include "FormFactorTable.h"
#include "SAXSProfile.h"
#include "Distribution.h"
#include <IMP/Particle.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

//! TODO: Not working, compile error.
//class IMPSAXSEXPORT
//DeltaDistributionFunction : public Distribution<algebra::Vector3D> {
class IMPSAXSEXPORT DeltaDistributionFunction {
public:
  DeltaDistributionFunction(Float bin_size, FormFactorTable* ff_table,
                            const std::vector<Particle*>& particles);

  friend class SAXSScore;

  void calculate_derivative_distribution(unsigned int iatom);

  //! computes distribution contribution from inter-molecular
  //! interactions between the particles
  void calculate_derivative_distribution(
                              const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  // ! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

  // ! get distribution as array of Float
  unsigned int size() const { return distribution_.size(); }
  std::vector<algebra::Vector3D> get_distribution() const{return distribution_;}
  Float get_max_pr_distance(void) { return max_pr_distance_; }
  Float get_bin_size_(void) { return bin_size_; }

private:
  void add_to_derivative_distribution(Float dist,
                                      const algebra::Vector3D &value);

  unsigned int dist2index(Float dist) const {
    return algebra::round( dist * one_over_bin_size_ );
  }

  Float index2dist(unsigned int index) const { return index * bin_size_; }

private:
  std::vector< algebra::Vector3D > distribution_;
  Float bin_size_, one_over_bin_size_; // resolution of discretization
  FormFactorTable* ff_table_; // pointer to form factors table
  Float max_pr_distance_;  // paramter for maximum r value for p(r) function
  std::vector < algebra::Vector3D > coordinates_;
  std::vector<Float> zero_formfactor_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DELTA_DISTRIBUTION_FUNCTION_H */

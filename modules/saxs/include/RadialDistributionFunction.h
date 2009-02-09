/**
 * \file RadialDistributionFunction.h \brief computes
 *
 * radial distribution function
 * required for SAXS profile computation
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_RADIAL_DISTRIBUTION_FUNCTION_H
#define IMPSAXS_RADIAL_DISTRIBUTION_FUNCTION_H

#include "config.h"
#include "FormFactorTable.h"
#include "SAXSProfile.h"
#include "Distribution.h"

#include <IMP/Particle.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

//! TODO: Not working, compile error.
//template<class ValueT>
//class IMPSAXSEXPORT RadialDistributionFunction : public Distribution<ValueT> {
class IMPSAXSEXPORT RadialDistributionFunction {

public:
  RadialDistributionFunction(Float bin_size, FormFactorTable* ff_table);

  friend class SAXSProfile;

  void calculate_distribution(const std::vector<Particle*>& particles);

  //! computes distribution contribution from inter-molecular
  // interactions between the particles
  void calculate_distribution(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  // ! get distribution as array of Float
  std::vector<Float> get_distribution() const { return distribution_; }

  // ! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

  Float get_max_pr_distance(void) { return max_pr_distance_; }
  Float get_bin_size_(void) { return bin_size_; }

private:
  void add_to_distribution(Float dist, Float value);

  unsigned int dist2index(Float dist) const {
    return algebra::round( dist/bin_size_ );
  }

  Float index2dist(unsigned int index) const { return index*bin_size_; }

private:
  Floats distribution_;
  Float bin_size_; // resolution of discretization
  FormFactorTable* ff_table_; // pointer to form factors table
  Float max_pr_distance_;  // paramter for maximum r value for p(r) function
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RADIAL_DISTRIBUTION_FUNCTION_H */

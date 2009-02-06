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

#include <IMP/Particle.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT RadialDistributionFunction {
public:
  RadialDistributionFunction(Float bin_size, FormFactorTable* ff_table);

  friend class SAXSProfile;

  void calculate_distribution(const std::vector<Particle*>& particles);

  //! computes distribution contribution from iter-molecular
  // interactions between the particles
  void calculate_distribution(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

 /* void calculate_derivative_distribution
                             (const std::vector<Particle*>& particles,
                std::vector<IMP::algebra::Vector3D> &derivative_distribution );
  */

  // ! get distribution as array of Float
  Floats get_distribution() const { return distribution_; }

  // ! print tables
  void show(std::ostream &out=std::cout, std::string prefix="") const;

  Float get_max_pr_distance(void) { return max_pr_distance_; }
  Float set_max_pr_distance(Float max_pr_distance) {
    max_pr_distance_ = max_pr_distance;
    distribution_.reserve(dist2index(max_pr_distance_));
    return max_pr_distance_;
  }

private:

  void add_to_distribution(Float dist, Float value);

/*  void add_to_derivative_distribution
        (Float dist, const algebra::Vector3D &value,
         std::vector<IMP::algebra::Vector3D> &derivative_distribution);
*/

  unsigned int dist2index(Float dist) const {
    return floor(dist/bin_size_ + 0.5);
  }

  Float index2dist(unsigned int index) const { return index*bin_size_; }

private:
  Floats distribution_;
  //std::vector<IMP::algebra::Vector3D> derivative_distribution_;
  Float bin_size_; // resolution of discretization
  FormFactorTable* ff_table_; // pointer to form factors table
  Float max_pr_distance_;  // paramter for maximum r value for p(r) function
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RADIAL_DISTRIBUTION_FUNCTION_H */

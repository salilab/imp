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

class RadialDistributionFunction {
public:
  RadialDistributionFunction(Float bin_size, FormFactorTable* ff_table);

  friend class SAXSProfile;

  void calculate_distribution(const std::vector<Particle*>& particles);

  //! computes distribution contribution from iter-molecular
  // interactions between the particles
  void calculate_distribution(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  // ! get distribution as array of floats
  const Floats& get_distribution() const { return distribution_; }

  // ! print
  void show(std::ostream &out, std::string prefix) const;
private:

  void add_to_distribution(Float dist, Float value);

  unsigned int dist2index(Float dist) const { return roundf(dist/bin_size_); }

  Float index2dist(unsigned int index) const { return index*bin_size_; }

private:
  Floats distribution_;
  Float bin_size_; // resolution of discretization
  FormFactorTable* ff_table_; // pointer to form factors table
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RADIAL_DISTRIBUTION_FUNCTION_H */

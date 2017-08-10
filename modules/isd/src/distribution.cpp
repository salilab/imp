/**
 *  \file distribution.cpp
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/distribution.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

double OneDimensionalDistribution::do_get_density(double v) const {
  return std::exp(-evaluate(v));
}

Floats OneDimensionalDistribution::do_evaluate(const Floats &vs) const {
  Floats ret;
  for (unsigned int i = 0; i < vs.size(); ++i) {
    ret.push_back(evaluate(vs[i]));
  }
  return ret;
}

Floats OneDimensionalDistribution::do_get_density(const Floats &vs) const {
  Floats ret;
  for (unsigned int i = 0; i < vs.size(); ++i) {
    ret.push_back(get_density(vs[i]));
  }
  return ret;
}

double OneDimensionalSufficientDistribution::do_get_density() const {
  return std::exp(-evaluate());
}


IMPISD_END_NAMESPACE

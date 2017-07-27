/**
 *  \file distribution.cpp
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/distribution.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

double OneDimensionalDistribution::do_get_density(double v) {
  return std::exp(-evaluate(v));
}


double OneDimensionalSufficientDistribution::do_evaluate(double v) {
  return evaluate(Floats(1, v));
}

double OneDimensionalSufficientDistribution::do_evaluate(Floats vs) {
  update_sufficient_statistics(vs);
  return evaluate();
}

double OneDimensionalSufficientDistribution::do_get_density() const {
  return std::exp(-evaluate());
}

double OneDimensionalSufficientDistribution::do_get_density(Floats vs) {
  return std::exp(-evaluate(vs));
}

IMPISD_END_NAMESPACE

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

}

}

double OneDimensionalSufficientDistribution::do_get_density() const {
  return std::exp(-evaluate());
}


IMPISD_END_NAMESPACE

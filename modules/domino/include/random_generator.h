/**
 *  \file RandomGenerator.h   \brief random number generator
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_RANDOM_GENERATOR_H
#define IMPDOMINO_RANDOM_GENERATOR_H

#include <cstdlib>
#include <math.h>
#include "IMP/random.h"
#include "config.h"
#include "IMP/macros.h"
#include  <boost/random/normal_distribution.hpp>
IMPDOMINO_BEGIN_NAMESPACE

//! Generate a random integer number
/**
/param[in] n , the range is [0,n-1]
 */
inline int random_int(int n) {
  ::boost::uniform_int<> rand(0,n-1);
  return rand(random_number_generator);
}
//! Generate a random number in the range [lo,hi]
inline double random_uniform(double lo=0.0, double hi=1.0) {
  ::boost::uniform_real<> rand(lo, hi);
  return rand(random_number_generator);
}
//! Gaussian random number generator
/** Returns a normally distributed with zero mean and unit variance
 */
inline double random_gauss(double mean=0.0,double sigma=1.0) {
  boost::normal_distribution<double> norm_dist(mean, sigma);
  return norm_dist(random_number_generator);
}

IMPDOMINO_END_NAMESPACE
#endif  /* IMPDOMINO_RANDOM_GENERATOR_H */

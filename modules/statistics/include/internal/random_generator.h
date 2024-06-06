/**
 *  \file random_generator.h   \brief random number generator
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_RANDOM_GENERATOR_H
#define IMPSTATISTICS_RANDOM_GENERATOR_H

#include "IMP/random.h"
#include <IMP/statistics/statistics_config.h>
#include "IMP/macros.h"
#include <boost/random/normal_distribution.hpp>
#include <boost/random.hpp>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

//! Generate a random integer number
/**
\param[in] n , the range is [0,n-1]
 */
inline int random_int(int n) {
  ::boost::random::uniform_int_distribution<> rand(0, n - 1);
  return rand(random_number_generator);
}
//! Generate a random number in the range [lo,hi]
inline double random_uniform(double lo = 0.0, double hi = 1.0) {
  ::boost::random::uniform_real_distribution<> rand(lo, hi);
  return rand(random_number_generator);
}
//! Gaussian random number generator
/** Returns a normally distributed with zero mean and unit variance
 */
inline double random_gauss(double mean = 0.0, double sigma = 1.0) {
  typedef boost::normal_distribution<double> NormalDistribution;

  NormalDistribution norm_dist(mean, sigma);
  boost::variate_generator<RandomNumberGenerator&, NormalDistribution>
      generator(random_number_generator, norm_dist);
  return generator();
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE

#endif /* IMPSTATISTICS_RANDOM_GENERATOR_H */

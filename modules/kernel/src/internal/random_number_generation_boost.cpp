/**
 *  \file IMP/kernel/internal/random_number_generation_boost.cpp
 *  \brief GPU or CPU pooled random number generation
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/random_number_generation_boost.h"
#include "IMP/random.h"
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_real.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

void get_random_numbers_normal_boost(double* p_random_array, unsigned int n,
                                  double mean, double stddev)
{
  boost::normal_distribution<double> nd(mean, stddev);
  for(unsigned int i=0; i<n; i++)
    {
      *(p_random_array++) = nd(IMP::random_number_generator);
    }
}

void get_random_numbers_normal_boost(float* p_random_array, unsigned int n,
                                  float mean, float stddev)
{
  boost::normal_distribution<float> nd(mean, stddev);
  for(unsigned int i=0; i<n; i++)
    {
      *(p_random_array++) = nd(IMP::random_number_generator);
    }
}

void get_random_numbers_uniform_boost(float* p_random_array, unsigned int n)
{
  boost::uniform_real<float> ud(0.0, 1.0);
  for(unsigned int i=0; i<n; i++)
    {
      *(p_random_array++) = ud(IMP::random_number_generator);
    }
}

void get_random_numbers_uniform_boost(double* p_random_array, unsigned int n)
{
  boost::uniform_real<double> ud(0.0, 1.0);
  for(unsigned int i=0; i<n; i++)
    {
      *(p_random_array++) = ud(IMP::random_number_generator);
    }
}

IMPKERNEL_END_INTERNAL_NAMESPACE

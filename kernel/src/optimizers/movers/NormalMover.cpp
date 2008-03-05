/**
 *  \file NormalMover.cpp
 *  \brief A modifier which perturbs a point with a gaussian.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <boost/random/normal_distribution.hpp>
#include "IMP/optimizers/movers/NormalMover.h"
#include "IMP/random.h"

namespace IMP
{

NormalMover::NormalMover(const Particles &pis,
                         const FloatKeys &vars,
                         Float max)
{
  add_particles(pis);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    add_key(vars[i]);
  }
  set_sigma(max);
}

void NormalMover::generate_move(float scale)
{
  boost::normal_distribution<double> mrng(0, stddev_);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<double> >
                          sampler(random_number_generator, mrng);

  for (unsigned int i = 0; i < number_of_particles(); ++i) {
    for (unsigned int j = 0; j < number_of_float_keys(); ++j) {
      float c = get_float(i, j);
      float r = sampler();
      // Check for NaN (x!=x when x==NaN) (can only use std::isnan with C99)
      IMP_assert(r == r, "Bad random");
      IMP_assert(c == c, "Bad stored");
      propose_value(i, j, c + r);
    }
  }
}

} // namespace IMP

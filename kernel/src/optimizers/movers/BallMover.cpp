/**
 *  \file BallMover.cpp  \brief A modifier which perturbs a discrete variable.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <boost/random/uniform_real.hpp>
#include "IMP/optimizers/movers/BallMover.h"
#include "IMP/random.h"

namespace IMP
{

// These functions probably should be exposed at some point

static void random_point_in_sphere(unsigned int D,
                                   Float radius,
                                   Float v[])
{
  IMP_assert(radius > 0, "No volume there");
  ::boost::uniform_real<> rand(-radius, radius);
  Float norm;
  do {
    norm = 0;
    for (unsigned int i = 0; i < D; ++i) {
      v[i] = rand(random_number_generator);
      norm += v[i] * v[i];
    }
  } while (norm > radius*radius);
}

static std::vector<Float>
random_point_in_sphere(const std::vector<Float> &center,
                       Float radius)
{
  IMP_assert(radius > 0, "No volume there");
  Float v[center.size()];
  random_point_in_sphere(center.size(), radius, v);
  std::vector<Float> r(center.size());
  for (unsigned int i = 0; i < center.size(); ++i) {
    r[i] = center[i] + v[i];
  }
  return r;
}



BallMover::BallMover(const Particles &pis,
                     const FloatKeys &vars,
                     Float max)
{
  add_particles(pis);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    add_key(vars[i]);
  }
  max_step_ = max;
}

void BallMover::generate_move(float scale)
{
  std::vector<Float> center(number_of_float_keys());
  for (unsigned int i = 0; i < number_of_particles(); ++i) {
    for (unsigned int j = 0; j < number_of_float_keys(); ++j) {
      center[j] = get_float(i, j);
    }
    std::vector<Float> npos = random_point_in_sphere(center, scale * max_step_);
    for (unsigned int j = 0; j < number_of_float_keys(); ++j) {
      propose_value(i, j, npos[j]);
    }
  }
}

} // namespace IMP

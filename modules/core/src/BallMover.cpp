/**
 *  \file BallMover.cpp  \brief A modifier which perturbs a discrete variable.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/BallMover.h>

#include <IMP/random.h>

#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE

// These functions probably should be exposed at some point

static void random_point_in_sphere(unsigned int D,
                                   Float radius,
                                   std::vector<Float> v)
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
  std::vector<Float> v(center.size());
  random_point_in_sphere(center.size(), radius, v);
  std::vector<Float> r(center.size());
  for (unsigned int i = 0; i < center.size(); ++i) {
    r[i] = center[i] + v[i];
  }
  return r;
}



BallMover::BallMover(SingletonContainer *sc,
                     const FloatKeys &vars,
                     Float max): MoverBase(sc)
{
  add_float_keys(vars);
  set_radius(max);
}

void BallMover::generate_move(Float scale)
{
  std::vector<Float> center(get_number_of_float_keys());
  for (unsigned int i = 0;
       i < get_container()->get_number_of_particles(); ++i) {
    for (unsigned int j = 0; j < get_number_of_float_keys(); ++j) {
      center[j] = get_float(i, j);
    }
    std::vector<Float> npos = random_point_in_sphere(center, scale * radius_);
    for (unsigned int j = 0; j < get_number_of_float_keys(); ++j) {
      propose_value(i, j, npos[j]);
    }
  }
}

IMPCORE_END_NAMESPACE

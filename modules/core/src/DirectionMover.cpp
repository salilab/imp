/**
 *  \file DirectionMover.cpp
 *  \brief A mover that transforms a Direction
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/DirectionMover.h>
#include <IMP/random.h>
#include <IMP/algebra/vector_generators.h>

IMPCORE_BEGIN_NAMESPACE

DirectionMover::DirectionMover(Model *m, ParticleIndex pi, Float max_rotation,
                               Float reflect_probability)
    : MonteCarloMover(m, m->get_particle(pi)->get_name() + " mover") {
  initialize(pi, max_rotation, reflect_probability);
}

DirectionMover::DirectionMover(Direction d, Float max_rotation,
                               Float reflect_probability)
    : MonteCarloMover(d->get_model(), d->get_name() + " mover") {
  initialize(d->get_index(), max_rotation, reflect_probability);
}

void DirectionMover::initialize(ParticleIndex pi, double max_rotation,
                                double reflect_probability) {
  pi_ = pi;
  set_maximum_rotation(max_rotation);
  set_reflect_probability(reflect_probability);
}

void DirectionMover::set_maximum_rotation(Float mr) {
  IMP_USAGE_CHECK(mr >= 0, "Max rotation must be positive");
  IMP_USAGE_CHECK(
    mr == 0 || get_direction().get_direction_is_optimized(),
    "Direction must be set to optimized in order to rotate.");
  max_angle_ = mr;
}

void DirectionMover::set_reflect_probability(Float rp) {
  IMP_USAGE_CHECK(rp >= 0 && rp <= 1,
                  "Reflection probability must be between 0 and 1.");
  IMP_USAGE_CHECK(
    rp == 0 || get_direction().get_direction_is_optimized(),
    "Direction must be set to optimized in order to reflect.");
  reflect_prob_ = rp;
}

MonteCarloMoverResult DirectionMover::do_propose() {
  Direction d(get_model(), pi_);
  last_direction_ = d.get_direction();

  if (max_angle_ > 0) {
    algebra::Vector3D axis =
        algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
    axis -= (axis * last_direction_) * last_direction_;
    ::boost::uniform_real<> rand1(-max_angle_, max_angle_);
    Float angle = rand1(random_number_generator);
    algebra::Rotation3D rt = algebra::get_rotation_about_axis(axis, angle);
    d.set_direction(rt.get_rotated(last_direction_));
  }

  if (reflect_prob_ > 0) {
    ::boost::uniform_real<> rand2(0, 1);
    Float test = rand2(random_number_generator);
    if (test < reflect_prob_) {
      d.reflect();
    }
  }

  return MonteCarloMoverResult(ParticleIndexes(1, pi_), 1.0);
}

void DirectionMover::do_reject() {
  Direction d(get_model(), pi_);
  d.set_direction(last_direction_);
}

ModelObjectsTemp DirectionMover::do_get_inputs() const {
  return ParticlesTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE

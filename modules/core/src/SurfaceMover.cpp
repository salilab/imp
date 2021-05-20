/**
 *  \file SurfaceMover.cpp
 *  \brief A mover that transforms a Surface
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/SurfaceMover.h>
#include <IMP/random.h>
#include <IMP/algebra/vector_generators.h>

IMPCORE_BEGIN_NAMESPACE

SurfaceMover::SurfaceMover(Model *m, ParticleIndex pi, Float max_translation,
                           Float max_rotation, Float reflect_probability)
    : MonteCarloMover(m, m->get_particle(pi)->get_name() + " mover") {
  initialize(pi, max_translation, max_rotation, reflect_probability);
}

SurfaceMover::SurfaceMover(Surface s, Float max_translation, Float max_rotation,
                           Float reflect_probability)
    : MonteCarloMover(s->get_model(), s->get_name() + " mover") {
  initialize(s->get_index(), max_translation, max_rotation,
             reflect_probability);
}

void SurfaceMover::initialize(ParticleIndex pi, double max_translation,
                              double max_rotation, double reflect_probability) {
  pi_ = pi;
  set_maximum_translation(max_translation);
  set_maximum_rotation(max_rotation);
  set_reflect_probability(reflect_probability);
}

void SurfaceMover::set_maximum_translation(Float mt) {
  IMP_USAGE_CHECK(mt >= 0, "Max translation must be positive");
  IMP_USAGE_CHECK(
    mt == 0 || get_surface().get_coordinates_are_optimized(),
    "Surface coordinates must be set to optimized in order to translate.");
  max_translation_ = mt;
}

void SurfaceMover::set_maximum_rotation(Float mr) {
  IMP_USAGE_CHECK(mr >= 0, "Max rotation must be positive");
  IMP_USAGE_CHECK(
    mr == 0 || get_surface().get_normal_is_optimized(),
    "Surface normal must be set to optimized in order to rotate.");
  max_angle_ = mr;
}

void SurfaceMover::set_reflect_probability(Float rp) {
  IMP_USAGE_CHECK(rp >= 0 && rp <= 1,
                  "Reflection probability must be between 0 and 1.");
  IMP_USAGE_CHECK(
    rp == 0 || get_surface().get_normal_is_optimized(),
    "Surface normal must be set to optimized in order to reflect.");
  reflect_prob_ = rp;
}

MonteCarloMoverResult SurfaceMover::do_propose() {
  Surface s(get_model(), pi_);
  last_transform_ = s.get_reference_frame().get_transformation_to();

  if (max_translation_ > 0) {
    algebra::Vector3D translation = algebra::get_random_vector_in(
        algebra::Sphere3D(s.get_coordinates(), max_translation_));
    s.set_coordinates(translation);
  }

  if (max_angle_ > 0) {
    algebra::Vector3D axis =
        algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
    ::boost::uniform_real<> rand1(-max_angle_, max_angle_);
    Float angle = rand1(random_number_generator);
    algebra::Rotation3D rt = algebra::get_rotation_about_axis(axis, angle);
    s.set_normal(rt.get_rotated(s.get_normal()));
  }

  if (reflect_prob_ > 0) {
    ::boost::uniform_real<> rand2(0, 1);
    Float test = rand2(random_number_generator);
    if (test < reflect_prob_) {
      s.reflect();
    }
  }

  return MonteCarloMoverResult(ParticleIndexes(1, pi_), 1.0);
}

void SurfaceMover::do_reject() {
  Surface s(get_model(), pi_);
  s.set_reference_frame(algebra::ReferenceFrame3D(last_transform_));
  last_transform_ = algebra::Transformation3D();
}

ModelObjectsTemp SurfaceMover::do_get_inputs() const {
  return ParticlesTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE

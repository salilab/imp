/**
 *  \file example/Gaussian.cpp
 *  \brief Add a name to a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Gaussian.h>
#include <math.h>
IMPCORE_BEGIN_NAMESPACE

FloatKey Gaussian::get_standard_deviation_key(unsigned int i) {
  static FloatKey keys[3] = {FloatKey("x standard deviation"),
                             FloatKey("y standard deviation"),
                             FloatKey("z standard deviation")};
  IMP_USAGE_CHECK(i < 3, "Out of range key");
  return keys[i];
}

void Gaussian::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                 const algebra::Gaussian3D &g) {
  core::RigidBody::setup_particle(m, pi, g.get_reference_frame());
  m->add_attribute(get_standard_deviation_key(0), pi,
                   g.get_standard_deviations()[0]);
  m->add_attribute(get_standard_deviation_key(1), pi,
                   g.get_standard_deviations()[1]);
  m->add_attribute(get_standard_deviation_key(2), pi,
                   g.get_standard_deviations()[2]);
}

algebra::Gaussian3D Gaussian::get_gaussian() const {
  return algebra::Gaussian3D(
      RigidBody::get_reference_frame(),
      algebra::Vector3D(
          get_model()->get_attribute(get_standard_deviation_key(0),
                                     get_particle_index()),
          get_model()->get_attribute(get_standard_deviation_key(1),
                                     get_particle_index()),
          get_model()->get_attribute(get_standard_deviation_key(2),
                                     get_particle_index())));
}

void Gaussian::set_gaussian(const algebra::Gaussian3D &g) {
  RigidBody::set_reference_frame(g.get_reference_frame());
  set_standard_deviations(g.get_standard_deviations());
}

void Gaussian::show(std::ostream &out) const {
  out << get_gaussian();
}

IMPCORE_END_NAMESPACE

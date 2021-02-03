/**
 *  \file MolecularDynamics.cpp  \brief 1-D and 3-D MD optimizer
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/MolecularDynamics.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>

#include <IMP/log.h>
#include <IMP/random.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPISD_BEGIN_NAMESPACE

namespace {
// Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
// and mass is in g/mol, conversion factor necessary to get accelerations
// in angstrom/fs/fs from raw derivatives
static const double deriv_to_acceleration = -4.1868e-4;
}

MolecularDynamics::MolecularDynamics(Model *m)
    : atom::MolecularDynamics(m) {
  vnuis_ = FloatKey("vel");
}

bool MolecularDynamics::get_is_simulation_particle(ParticleIndex pi)
    const {
  Particle *p = get_model()->get_particle(pi);
  bool ret = IMP::core::XYZ::get_is_setup(p) &&
             IMP::core::XYZ(p).get_coordinates_are_optimized() &&
             atom::Mass::get_is_setup(p);
  bool ret2 = IMP::isd::Nuisance::get_is_setup(p) &&
              IMP::isd::Nuisance(p).get_nuisance_is_optimized() &&
              atom::Mass::get_is_setup(p);
  IMP_USAGE_CHECK(!(ret && ret2), "Particle " << p
                                              << " is both XYZ and Nuisance!");

  if (ret) {
    if (!atom::LinearVelocity::get_is_setup(p)) {
      atom::LinearVelocity::setup_particle(p);
    }
  }
  if (ret2) {
    if (!p->has_attribute(vnuis_)) {
      p->add_attribute(vnuis_, 0.0, false);
    }
  }
  return ret || ret2;
}

void MolecularDynamics::setup_degrees_of_freedom(
    const ParticleIndexes &ps) {
  atom::MolecularDynamics::setup_degrees_of_freedom(ps);

  unsigned dof_nuisances = 0;
  for (unsigned i = 0; i < ps.size(); i++) {
    Particle *p = get_model()->get_particle(ps[i]);
    if (Nuisance::get_is_setup(p)) dof_nuisances += 1;
  }
  degrees_of_freedom_ -= 2 * dof_nuisances;
}

void MolecularDynamics::propagate_coordinates(const ParticleIndexes &ps,
                                              double ts) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float invmass = 1.0 / atom::Mass(get_model(), ps[i]).get_mass();
    Particle *p = get_model()->get_particle(ps[i]);
    if (Nuisance::get_is_setup(p)) {
      Nuisance d(p);

      Float coord = d.get_nuisance();
      Float dcoord = d.get_nuisance_derivative();

      // calculate velocity at t+(delta t/2) from that at t
      Float velocity = get_model()->get_attribute(vnuis_, ps[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;

      cap_velocity_component(velocity);
      get_model()->set_attribute(vnuis_, ps[i], velocity);

      // calculate position at t+(delta t) from that at t
      coord += velocity * ts;
      d.set_nuisance(coord);

    } else {
      core::XYZ d(p);
      atom::LinearVelocity v(p);
      algebra::Vector3D coord = d.get_coordinates();
      algebra::Vector3D dcoord = d.get_derivatives();

      // calculate velocity at t+(delta t/2) from that at t
      algebra::Vector3D velocity = v.get_velocity();
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;
      for (unsigned j = 0; j < 3; ++j) {
        cap_velocity_component(velocity[j]);
      }
      v.set_velocity(velocity);

      // calculate position at t+(delta t) from that at t
      coord += velocity * ts;
      d.set_coordinates(coord);
    }
  }
}

void MolecularDynamics::propagate_velocities(const ParticleIndexes &ps,
                                             double ts) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float invmass = 1.0 / atom::Mass(get_model(), ps[i]).get_mass();
    Particle *p = get_model()->get_particle(ps[i]);
    if (Nuisance::get_is_setup(p)) {
      Nuisance d(p);
      Float dcoord = d.get_nuisance_derivative();

      // calculate velocity at t+(delta t) from that at t+(delta t/2)
      Float velocity = get_model()->get_attribute(vnuis_, ps[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;
      get_model()->set_attribute(vnuis_, ps[i], velocity);

    } else {
      core::XYZ d(p);
      algebra::Vector3D dcoord = d.get_derivatives();
      atom::LinearVelocity v(p);
      // calculate velocity at t+(delta t) from that at t+(delta t/2)
      algebra::Vector3D velocity = v.get_velocity();
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;
      v.set_velocity(velocity);
    }
  }
}

Float MolecularDynamics::get_kinetic_energy() const {
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mafs in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  ParticlesTemp ps = get_simulation_particles();
  for (ParticlesTemp::iterator iter = ps.begin(); iter != ps.end();
       ++iter) {
    Particle *p = *iter;
    Float mass = atom::Mass(p).get_mass();
    if (Nuisance::get_is_setup(p)) {
      Float vel = p->get_value(vnuis_);
      ekinetic += mass * vel * vel;
    } else {
      algebra::Vector3D v = atom::LinearVelocity(p).get_velocity();
      ekinetic += mass * (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    }
  }
  return 0.5 * ekinetic * conversion;
}

void MolecularDynamics::assign_velocities(Float temperature) {
  ParticleIndexes ips = get_simulation_particle_indexes();
  setup_degrees_of_freedom(ips);
  ParticlesTemp ps = IMP::internal::get_particle(get_model(), ips);

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<RandomNumberGenerator &,
                           boost::normal_distribution<Float> >
      sampler(random_number_generator, mrng);

  for (ParticlesTemp::iterator iter = ps.begin(); iter != ps.end();
       ++iter) {
    Particle *p = *iter;
    if (Nuisance::get_is_setup(p)) {
      p->set_value(vnuis_, sampler());
    } else {
      atom::LinearVelocity(p).set_velocity(algebra::Vector3D(sampler(),
                                                 sampler(), sampler()));
    }
  }

  Float rescale =
      sqrt(temperature / get_kinetic_temperature(get_kinetic_energy()));

  for (ParticlesTemp::iterator iter = ps.begin(); iter != ps.end();
       ++iter) {
    Particle *p = *iter;
    if (Nuisance::get_is_setup(p)) {
      Float velocity = p->get_value(vnuis_);
      velocity *= rescale;
      p->set_value(vnuis_, velocity);
    } else {
      atom::LinearVelocity v(p);

      algebra::Vector3D velocity = v.get_velocity();
      velocity *= rescale;
      v.set_velocity(velocity);
    }
  }
}

IMPISD_END_NAMESPACE

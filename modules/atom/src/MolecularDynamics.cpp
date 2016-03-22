/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/RemoveRigidMotionOptimizerState.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>

#include <IMP/log.h>
#include <IMP/random.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPATOM_BEGIN_NAMESPACE

namespace {
// Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
// and mass is in g/mol, conversion factor necessary to get accelerations
// in angstrom/fs/fs from raw derivatives
static const double deriv_to_acceleration = -4.1868e-4;
}

void LinearVelocity::show(std::ostream &out) const {
  out << "Linear velocity " << get_velocity() << std::endl;
}

void AngularVelocity::show(std::ostream &out) const {
  out << "Angular velocity " << get_velocity() << std::endl;
}

MolecularDynamics::MolecularDynamics(Model *m)
    : Simulator(m, "MD %1%") {
  initialize();
}

void MolecularDynamics::initialize() {
  set_maximum_time_step(4.0);
  degrees_of_freedom_ = 0;
  velocity_cap_ = std::numeric_limits<Float>::max();
}

bool MolecularDynamics::get_is_simulation_particle(ParticleIndex pi)
    const {
  Particle *p = get_model()->get_particle(pi);
  bool ret = IMP::core::XYZ::get_is_setup(p) &&
             IMP::core::XYZ(p).get_coordinates_are_optimized() &&
             Mass::get_is_setup(p);
  if (ret) {
    IMP_LOG_VERBOSE(p->get_name() << " is md particle" << std::endl);
    if (!LinearVelocity::get_is_setup(p)) {
      LinearVelocity::setup_particle(p);
    }
  }
  return ret;
}

void MolecularDynamics::setup(const ParticleIndexes &ps) {
  // Get starting score and derivatives, for first dynamics step velocities
  get_scoring_function()->evaluate(true);

  setup_degrees_of_freedom(ps);
}

void MolecularDynamics::setup_degrees_of_freedom(
    const ParticleIndexes &ps) {
  degrees_of_freedom_ = 3 * ps.size();

  // If global rotation and translation have been removed, reduce degrees
  // of freedom accordingly (kind of ugly...)
  for (OptimizerStateIterator o = optimizer_states_begin();
       o != optimizer_states_end(); ++o) {
    OptimizerState *os = *o;
    if (dynamic_cast<RemoveRigidMotionOptimizerState *>(os)) {
      degrees_of_freedom_ -= 6;
      break;
    }
  }
}

//! Perform a single dynamics step.
double MolecularDynamics::do_step(const ParticleIndexes &ps,
                                  double ts) {
  IMP_OBJECT_LOG;
  // Get coordinates at t+(delta t) and velocities at t+(delta t/2)
  propagate_coordinates(ps, ts);

  // Get derivatives at t+(delta t)
  get_scoring_function()->evaluate(true);

  // Get velocities at t+(delta t)
  propagate_velocities(ps, ts);

  return ts;
}

void MolecularDynamics::propagate_coordinates(const ParticleIndexes &ps,
                                              double ts) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float invmass = 1.0 / Mass(get_model(), ps[i]).get_mass();
    core::XYZ d(get_model(), ps[i]);
    LinearVelocity v(get_model(), ps[i]);
    algebra::Vector3D coord = d.get_coordinates();
    algebra::Vector3D dcoord = d.get_derivatives();
    // calculate velocity at t+(delta t/2) from that at t
    algebra::Vector3D velocity = v.get_velocity();
    velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;
    for (unsigned int j = 0; j < 3; ++j) {
      cap_velocity_component(velocity[j]);
    }
    v.set_velocity(velocity);

    // calculate position at t+(delta t) from that at t
    coord += velocity * ts;
    d.set_coordinates(coord);
  }
}

void MolecularDynamics::propagate_velocities(const ParticleIndexes &ps,
                                             double ts) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float invmass = 1.0 / Mass(get_model(), ps[i]).get_mass();
    core::XYZ d(get_model(), ps[i]);
    algebra::Vector3D dcoord = d.get_derivatives();
    LinearVelocity v(get_model(), ps[i]);
    // calculate velocity at t+(delta t) from that at t+(delta t/2)
    algebra::Vector3D velocity = v.get_velocity();
    velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;
    v.set_velocity(velocity);
  }
}

Float MolecularDynamics::get_kinetic_energy() const {
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mass in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  ParticlesTemp ps = get_simulation_particles();
  for (ParticlesTemp::iterator iter = ps.begin(); iter != ps.end();
       ++iter) {
    Particle *p = *iter;
    algebra::Vector3D v = LinearVelocity(p).get_velocity();
    Float mass = Mass(p).get_mass();

    ekinetic += mass * (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  }
  return 0.5 * ekinetic * conversion;
}

Float MolecularDynamics::get_kinetic_temperature(Float ekinetic) const {
  if (degrees_of_freedom_ == 0) {
    return 0.;
  } else {
    // E = (n/2)kT  n=degrees of freedom, k = Boltzmann constant
    // Boltzmann constant, in kcal/mol
    const Float boltzmann = 8.31441 / 4186.8;
    return 2.0 * ekinetic / (degrees_of_freedom_ * boltzmann);
  }
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
    LinearVelocity(p).set_velocity(algebra::Vector3D(sampler(), sampler(),
                                                     sampler()));
  }

  Float rescale =
      sqrt(temperature / get_kinetic_temperature(get_kinetic_energy()));

  for (ParticlesTemp::iterator iter = ps.begin(); iter != ps.end();
       ++iter) {
    Particle *p = *iter;
    LinearVelocity v(p);

    algebra::Vector3D velocity = v.get_velocity();
    velocity *= rescale;
    v.set_velocity(velocity);
  }
}

IMPATOM_END_NAMESPACE

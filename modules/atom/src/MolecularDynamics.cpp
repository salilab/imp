/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/RemoveRigidMotionOptimizerState.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>

#include <IMP/base/log.h>
#include <IMP/base/random.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPATOM_BEGIN_NAMESPACE

namespace {
// Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
// and mass is in g/mol, conversion factor necessary to get accelerations
// in angstrom/fs/fs from raw derivatives
static const double deriv_to_acceleration = -4.1868e-4;
}

MolecularDynamics::MolecularDynamics(kernel::Model *m)
  : Simulator(m, "MD %1%") {
  initialize();
}

void MolecularDynamics::initialize() {
  set_maximum_time_step(4.0);
  degrees_of_freedom_ = 0;
  velocity_cap_ = std::numeric_limits<Float>::max();
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

bool
MolecularDynamics::get_is_simulation_particle(kernel::ParticleIndex pi) const {
  kernel::Particle *p = get_model()->get_particle(pi);
  bool ret = IMP::core::XYZ::get_is_setup(p) &&
             IMP::core::XYZ(p).get_coordinates_are_optimized() &&
             Mass::get_is_setup(p);
  if (ret) {
    IMP_LOG_VERBOSE(p->get_name() << " is md particle" << std::endl);
    for (unsigned int i = 0; i < 3; ++i) {
      if (!p->has_attribute(vs_[i])) {
        p->add_attribute(vs_[i], 0.0, false);
      }
    }
  }
  return ret;
}

void MolecularDynamics::setup(const kernel::ParticleIndexes &ps) {
  // Get starting score and derivatives, for first dynamics step velocities
  get_scoring_function()->evaluate(true);

  setup_degrees_of_freedom(ps);
}

void
MolecularDynamics::setup_degrees_of_freedom(const kernel::ParticleIndexes &ps) {
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
double
MolecularDynamics::do_step(const kernel::ParticleIndexes &ps, double ts) {
  IMP_OBJECT_LOG;
  // Get coordinates at t+(delta t) and velocities at t+(delta t/2)
  propagate_coordinates(ps, ts);

  // Get derivatives at t+(delta t)
  get_scoring_function()->evaluate(true);

  // Get velocities at t+(delta t)
  propagate_velocities(ps, ts);

  return ts;
}

void MolecularDynamics::propagate_coordinates(const kernel::ParticleIndexes &ps,
                                              double ts) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float invmass = 1.0 / Mass(get_model(), ps[i]).get_mass();
    for (unsigned j = 0; j < 3; ++j) {
      core::XYZ d(get_model(), ps[i]);

      Float coord = d.get_coordinate(j);
      Float dcoord = d.get_derivative(j);

      // calculate velocity at t+(delta t/2) from that at t
      Float velocity = get_model()->get_attribute(vs_[j], ps[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;

      cap_velocity_component(velocity);
      get_model()->set_attribute(vs_[j], ps[i], velocity);

      // calculate position at t+(delta t) from that at t
      coord += velocity * ts;
      d.set_coordinate(j, coord);
    }
  }
}

void MolecularDynamics::propagate_velocities(const kernel::ParticleIndexes &ps,
                                             double ts) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float invmass = 1.0 / Mass(get_model(), ps[i]).get_mass();
    for (unsigned j = 0; j < 3; ++j) {
      core::XYZ d(get_model(), ps[i]);
      Float dcoord = d.get_derivative(j);

      // calculate velocity at t+(delta t) from that at t+(delta t/2)
      Float velocity = get_model()->get_attribute(vs_[j], ps[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;

      get_model()->set_attribute(vs_[j], ps[i], velocity);
    }
  }
}

Float MolecularDynamics::get_kinetic_energy() const {
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mass in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  kernel::ParticlesTemp ps = get_simulation_particles();
  for (kernel::ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    kernel::Particle *p = *iter;
    Float vx = p->get_value(vs_[0]);
    Float vy = p->get_value(vs_[1]);
    Float vz = p->get_value(vs_[2]);
    Float mass = Mass(p).get_mass();

    ekinetic += mass * (vx * vx + vy * vy + vz * vz);
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
  kernel::ParticleIndexes ips = get_simulation_particle_indexes();
  setup_degrees_of_freedom(ips);
  kernel::ParticlesTemp ps = IMP::internal::get_particle(get_model(), ips);

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<base::RandomNumberGenerator &,
                           boost::normal_distribution<Float> > sampler(
      base::random_number_generator, mrng);

  for (kernel::ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    kernel::Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      p->set_value(vs_[i], sampler());
    }
  }

  Float rescale =
      sqrt(temperature / get_kinetic_temperature(get_kinetic_energy()));

  for (kernel::ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    kernel::Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
    }
  }
}

IMPATOM_END_NAMESPACE

/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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


MolecularDynamics::MolecularDynamics(Model *m): Simulator(m, "MD %1%")
{
  initialize();
}


void MolecularDynamics::initialize() {
  set_maximum_time_step(4.0);
  degrees_of_freedom_=0;
  velocity_cap_=std::numeric_limits<Float>::max();
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}


bool MolecularDynamics::get_is_simulation_particle(Particle *p) const {
  bool ret=IMP::core::XYZ::particle_is_instance(p)
    && IMP::core::XYZ(p).get_coordinates_are_optimized()
    && Mass::particle_is_instance(p);
  if (ret) {
    for (unsigned int i=0; i< 3; ++i) {
      if (!p->has_attribute(vs_[i])) {
        p->add_attribute(vs_[i], 0.0, false);
      }
    }
  }
  return ret;
}


void MolecularDynamics::setup(const ParticlesTemp &ps)
{
  // Get starting score and derivatives, for first dynamics step velocities
  evaluate(true);

  setup_degrees_of_freedom(ps);
}


void MolecularDynamics::setup_degrees_of_freedom(const ParticlesTemp &ps)
{
  degrees_of_freedom_ = 3*ps.size();

  // If global rotation and translation have been removed, reduce degrees
  // of freedom accordingly (kind of ugly...)
  for (OptimizerStateIterator o = optimizer_states_begin();
       o != optimizer_states_end(); ++o) {
    if (dynamic_cast<RemoveRigidMotionOptimizerState *>(*o)) {
      degrees_of_freedom_ -= 6;
      break;
    }
  }
}

//! Perform a single dynamics step.
double MolecularDynamics::do_step(const ParticlesTemp &ps,
                                  double ts)
{
  // Get coordinates at t+(delta t) and velocities at t+(delta t/2)
  propagate_coordinates(ps, ts);

  // Get derivatives at t+(delta t)
  evaluate(true);

  // Get velocities at t+(delta t)
  propagate_velocities(ps, ts);

  return ts;
}

void MolecularDynamics::propagate_coordinates(const ParticlesTemp &ps,
                                              double ts)
{
  for (ParticlesTemp::const_iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    Float invmass = 1.0 / Mass(p).get_mass();
    for (unsigned i = 0; i < 3; ++i) {
      core::XYZ d(p);

      Float coord = d.get_coordinate(i);
      Float dcoord = d.get_derivative(i);

      // calculate velocity at t+(delta t/2) from that at t
      Float velocity = p->get_value(vs_[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;

      cap_velocity_component(velocity);
      p->set_value(vs_[i], velocity);

      // calculate position at t+(delta t) from that at t
      coord += velocity * ts;
      d.set_coordinate(i, coord);
    }
  }
}

void MolecularDynamics::propagate_velocities(const ParticlesTemp &ps,
                                             double ts)
{
  for (ParticlesTemp::const_iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    Float invmass = 1.0 / Mass(p).get_mass();
    for (unsigned i = 0; i < 3; ++i) {
      core::XYZ d(p);
      Float dcoord = d.get_derivative(i);

      // calculate velocity at t+(delta t) from that at t+(delta t/2)
      Float velocity = p->get_value(vs_[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;

      p->set_value(vs_[i], velocity);
    }
  }
}

Float MolecularDynamics::get_kinetic_energy() const
{
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mass in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  ParticlesTemp ps=get_simulation_particles();
  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    Float vx = p->get_value(vs_[0]);
    Float vy = p->get_value(vs_[1]);
    Float vz = p->get_value(vs_[2]);
    Float mass = Mass(p).get_mass();

    ekinetic += mass * (vx * vx + vy * vy + vz * vz);
  }
  return 0.5 * ekinetic * conversion;
}

Float MolecularDynamics::get_kinetic_temperature(Float ekinetic) const
{
  if (degrees_of_freedom_ == 0) {
    return 0.;
  } else {
    // E = (n/2)kT  n=degrees of freedom, k = Boltzmann constant
    // Boltzmann constant, in kcal/mol
    const Float boltzmann = 8.31441 / 4186.8;
    return 2.0 * ekinetic / (degrees_of_freedom_ * boltzmann);
  }
}



void MolecularDynamics::assign_velocities(Float temperature)
{
  ParticlesTemp ps=get_simulation_particles();
  setup_degrees_of_freedom(ps);

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
      sampler(random_number_generator, mrng);

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      p->set_value(vs_[i], sampler());
    }
  }

  Float rescale = sqrt(temperature/
                  get_kinetic_temperature(get_kinetic_energy()));

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
    }

  }
}


IMPATOM_END_NAMESPACE

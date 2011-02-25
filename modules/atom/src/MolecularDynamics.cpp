/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/MolecularDynamics.h>
#include <IMP/core/XYZ.h>

#include <IMP/log.h>
#include <IMP/random.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPATOM_BEGIN_NAMESPACE

MolecularDynamics::MolecularDynamics()
{
  initialize();
}

MolecularDynamics::MolecularDynamics(Model *m)
{
  initialize();
  set_model(m);
}


void MolecularDynamics::initialize() {
  time_step_=4.0;
  degrees_of_freedom_=0;
  velocity_cap_=std::numeric_limits<Float>::max();
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

void MolecularDynamics::do_show(std::ostream &) const {
}

IMP_LIST_IMPL(MolecularDynamics, Particle, particle, Particle*,
              Particles,
              {
                if (0) std::cout << index;
                for (unsigned int i=0; i< 3; ++i) {
                  if (!obj->has_attribute(vs_[i])) {
                    obj->add_attribute(vs_[i], 0.0, false);
                  }
                }
              },{},{});


void MolecularDynamics::setup_particles()
{
  degrees_of_freedom_ = 0;
  clear_particles();

  for (Model::ParticleIterator it= get_model()->particles_begin();
       it != get_model()->particles_end(); ++it) {
    Particle *p= *it;
    if (core::XYZ::particle_is_instance(p)
        && core::XYZ(p).get_coordinates_are_optimized()
        && Mass::particle_is_instance(p)) {
      add_particle(p);
      degrees_of_freedom_ += 3;
    }
  }
  degrees_of_freedom_ -= 6;
}


//! First part of velocity verlet
void MolecularDynamics::step_1()
{
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  static const Float deriv_to_acceleration = -4.1868e-4;

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    Float invmass = 1.0 / Mass(p).get_mass();
    for (unsigned i = 0; i < 3; ++i) {
      core::XYZ d(p);
      Float coord = d.get_coordinate(i);
      Float dcoord = d.get_derivative(i);
      Float velocity = p->get_value(vs_[i]);

//    calculate position at t+(delta t) from that at t
      coord += time_step_ * velocity +
               0.5 * time_step_ * time_step_ * dcoord
               * deriv_to_acceleration * invmass;
      d.set_coordinate(i, coord);

//    calculate velocity at t+(delta t/2) from that at t
      velocity += 0.5 * time_step_ * dcoord * deriv_to_acceleration * invmass;

      //cap_velocity_component(velocity);
      p->set_value(vs_[i], velocity);

    }
  }
}


//! Second part of velocity verlet
void MolecularDynamics::step_2()
{
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  static const Float deriv_to_acceleration = -4.1868e-4;

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    Float invmass = 1.0 / Mass(p).get_mass();
    core::XYZ d(p);
    for (unsigned i = 0; i < 3; ++i) {
      Float dcoord = d.get_derivative(i);
      Float velocity = p->get_value(vs_[i]);


      // calculate velocity at t+delta t from that at t+(delta t/2)
      velocity += 0.5 * time_step_ * dcoord * deriv_to_acceleration * invmass;

      //cap_velocity_component(velocity);
      p->set_value(vs_[i], velocity);

    }
  }
}


double MolecularDynamics::do_optimize(unsigned int max_steps)
{
  setup_particles();

  // get initial system score
  Float score = evaluate(true);

  for (unsigned int i = 0; i < max_steps; ++i) {
    update_states();
    step_1();
    score = evaluate(true);
    step_2();
    update_states();
  }
  return score;
}

Float MolecularDynamics::get_kinetic_energy() const
{
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mass in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  for (ParticleConstIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
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

  setup_particles();

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
      sampler(random_number_generator, mrng);

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      p->set_value(vs_[i], sampler());
    }
  }

  Float rescale = sqrt(temperature/
                  get_kinetic_temperature(get_kinetic_energy()));

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
    }

  }
}


IMPATOM_END_NAMESPACE

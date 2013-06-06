/**
 *  \file MolecularDynamics.cpp  \brief 1-D and 3-D MD optimizer
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/MolecularDynamics.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>

#include <IMP/base/log.h>
#include <IMP/base/random.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPISD_BEGIN_NAMESPACE

namespace {
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  static const double deriv_to_acceleration = -4.1868e-4;
}


MolecularDynamics::MolecularDynamics(Model *m): atom::MolecularDynamics(m)
{
  vnuis_ = FloatKey("vel");
}




bool MolecularDynamics::get_is_simulation_particle(ParticleIndex pi) const
{
  Particle *p=get_model()->get_particle(pi);
  bool ret=IMP::core::XYZ::particle_is_instance(p)
    && IMP::core::XYZ(p).get_coordinates_are_optimized()
    && atom::Mass::particle_is_instance(p);
  bool ret2=IMP::isd::Nuisance::particle_is_instance(p)
    && IMP::isd::Nuisance(p).get_nuisance_is_optimized()
    && atom::Mass::particle_is_instance(p);
  IMP_USAGE_CHECK(!(ret && ret2), "Particle "<<p<<" is both XYZ and Nuisance!");

  if (ret) {
    for (unsigned int i=0; i< 3; ++i) {
      if (!p->has_attribute(vs_[i])) {
        p->add_attribute(vs_[i], 0.0, false);
      }
    }
  }
  if (ret2) {
      if (!p->has_attribute(vnuis_)) {
          p->add_attribute(vnuis_, 0.0, false);
      }
  }
  return ret || ret2;
}

void MolecularDynamics::setup_degrees_of_freedom(const ParticleIndexes &ps)
{
  atom::MolecularDynamics::setup_degrees_of_freedom(ps);

  unsigned dof_nuisances = 0;
  for (unsigned i = 0; i<ps.size(); i++)
  {
      Particle *p=get_model()->get_particle(ps[i]);
      if (Nuisance::particle_is_instance(p)) dof_nuisances += 1;
  }
  degrees_of_freedom_ -= 2*dof_nuisances;
}

void MolecularDynamics::propagate_coordinates(const ParticleIndexes &ps,
                                              double ts)
{
  for (unsigned int i=0; i< ps.size(); ++i) {
    Float invmass = 1.0 / atom::Mass(get_model(), ps[i]).get_mass();
    Particle *p=get_model()->get_particle(ps[i]);
    if (Nuisance::particle_is_instance(p))
    {
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
        for (unsigned j = 0; j < 3; ++j) {
          core::XYZ d(p);

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
}

void MolecularDynamics::propagate_velocities(const ParticleIndexes &ps,
                                             double ts)
{
  for (unsigned int i=0; i< ps.size(); ++i) {
    Float invmass = 1.0 / atom::Mass(get_model(), ps[i]).get_mass();
    Particle *p=get_model()->get_particle(ps[i]);
    if (Nuisance::particle_is_instance(p))
    {
       Nuisance d(p);
       Float dcoord = d.get_nuisance_derivative();

       // calculate velocity at t+(delta t) from that at t+(delta t/2)
       Float velocity = get_model()->get_attribute(vnuis_, ps[i]);
       velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;
       get_model()->set_attribute(vnuis_, ps[i], velocity);

    } else {
        core::XYZ d(p);
        for (unsigned j = 0; j < 3; ++j) {
          Float dcoord = d.get_derivative(j);

          // calculate velocity at t+(delta t) from that at t+(delta t/2)
          Float velocity = get_model()->get_attribute(vs_[j], ps[i]);
          velocity += 0.5 * dcoord * deriv_to_acceleration * invmass * ts;

          get_model()->set_attribute(vs_[j], ps[i], velocity);
        }
    }
  }
}

Float MolecularDynamics::get_kinetic_energy() const
{
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mafs in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  ParticlesTemp ps=get_simulation_particles();
  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    Float mass = atom::Mass(p).get_mass();
    if (Nuisance::particle_is_instance(p)) {
        Float vel = p->get_value(vnuis_);
        ekinetic += mass * vel*vel;
    } else {
        Float vx = p->get_value(vs_[0]);
        Float vy = p->get_value(vs_[1]);
        Float vz = p->get_value(vs_[2]);
        ekinetic += mass * (vx * vx + vy * vy + vz * vz);
    }
  }
  return 0.5 * ekinetic * conversion;
}


void MolecularDynamics::assign_velocities(Float temperature)
{
  ParticleIndexes ips=get_simulation_particle_indexes();
  setup_degrees_of_freedom(ips);
  ParticlesTemp ps= IMP::internal::get_particle(get_model(), ips);

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<base::RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
      sampler(base::random_number_generator, mrng);

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    if (Nuisance::particle_is_instance(p)) {
        p->set_value(vnuis_, sampler());
    } else {
        for (int i = 0; i < 3; ++i) {
          p->set_value(vs_[i], sampler());
        }
    }
  }

  Float rescale = sqrt(temperature/
                  get_kinetic_temperature(get_kinetic_energy()));

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    if (Nuisance::particle_is_instance(p)) {
      Float velocity = p->get_value(vnuis_);
      velocity *= rescale;
      p->set_value(vnuis_, velocity);
    }else{
        for (int i = 0; i < 3; ++i) {
          Float velocity = p->get_value(vs_[i]);
          velocity *= rescale;
          p->set_value(vs_[i], velocity);
        }
    }

  }
}


IMPISD_END_NAMESPACE

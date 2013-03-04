/**
 *  \file Simulator.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/log.h>
#include <IMP/atom/Simulator.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/internal/units.h>
#include <boost/progress.hpp>
#include <boost/scoped_ptr.hpp>
#include <IMP/atom/constants.h>

IMPATOM_BEGIN_NAMESPACE

Simulator::Simulator(Model *m,
                     std::string name): Optimizer(m, name) {
  temperature_=strip_units(IMP::internal::DEFAULT_TEMPERATURE);
  max_time_step_=2;
  current_time_=0;
  last_time_step_=-1;
}


double Simulator::simulate(double time) {
  IMP_FUNCTION_LOG;
  set_was_used(true);
  ParticleIndexes ps= get_simulation_particle_indexes();

  setup(ps);
  double target= current_time_+time;
  boost::scoped_ptr<boost::progress_display> pgs;
  if (get_log_level()== PROGRESS) {
    pgs.reset(new boost::progress_display(time/max_time_step_));
  }
  while (current_time_ < target) {
    last_time_step_= do_step(ps, max_time_step_);
    current_time_+= last_time_step_;
    update_states();
    if (get_log_level()== PROGRESS) {
      ++(*pgs);
    }
  }
  return Optimizer::get_scoring_function()->evaluate(false);
}

ParticleIndexes Simulator::get_simulation_particle_indexes() const {
  IMP_FUNCTION_LOG;
  ParticleIndexes ps;
  if (get_number_of_particles()==0) {
    for (Model::ParticleIterator it = get_model()->particles_begin();
       it != get_model()->particles_end(); ++it) {
      if (get_is_simulation_particle((*it)->get_index())) {
        ps.push_back((*it)->get_index());
      }
    }
  } else {
    ps= IMP::internal::get_index(ParticlesTemp(particles_begin(),
                                               particles_end()));
  }
  return ps;
}

ParticlesTemp Simulator::get_simulation_particles() const {
  ParticleIndexes p= get_simulation_particle_indexes();
  return IMP::internal::get_particle(get_model(), p);
}

double Simulator::do_optimize(unsigned int ns) {
  return simulate(ns*max_time_step_);
}

void Simulator::do_show(std::ostream &out) const {
  out << " time step: " << max_time_step_ << std::endl;
  out << " temperature: " << temperature_ << std::endl;
}
double Simulator::get_kt() const {
  return IMP::atom::get_kt(get_temperature());
}

IMP_LIST_IMPL(Simulator, Particle, particle, Particle*,
              Particles);



double get_energy_in_femto_joules(double energy_in_kcal_per_mol) {
  unit::KilocaloriePerMol cforce(energy_in_kcal_per_mol);
  unit::Femtojoule nforce
    = unit::convert_Cal_to_J(cforce/unit::ATOMS_PER_MOL);
  return nforce.get_value();
}

double
get_force_in_femto_newtons(double f) {
  unit::KilocaloriePerAngstromPerMol cforce( f);
  unit::Femtonewton nforce
    = unit::convert_Cal_to_J(cforce/unit::ATOMS_PER_MOL);
  return nforce.get_value();
}

double
get_spring_constant_in_femto_newtons_per_angstrom(double
                    k) {
  // cheating a bit
  unit::KilocaloriePerAngstromPerMol cforce( k);
  unit::Femtonewton nforce
    = unit::convert_Cal_to_J(cforce/unit::ATOMS_PER_MOL);
  return nforce.get_value();
}



IMPATOM_END_NAMESPACE

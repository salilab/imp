/**
 *  \file Simulator.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Simulator.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/container_helpers.h>

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
  while (current_time_ < target) {
    last_time_step_= do_step(ps, max_time_step_);
    current_time_+= last_time_step_;
    update_states();
  }
  return Optimizer::evaluate(false);
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


IMP_LIST_IMPL(Simulator, Particle, particle, Particle*,
              Particles);
IMPATOM_END_NAMESPACE

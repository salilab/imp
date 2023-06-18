/**
 *  \file Simulator.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/log.h>
#include <IMP/atom/Simulator.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/internal/units.h>
#include <IMP/internal/BoostProgressDisplay.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/atom/constants.h>

IMPATOM_BEGIN_NAMESPACE

Simulator::Simulator(Model *m, std::string name, double wave_factor)
    : Optimizer(m, name), wave_factor_(wave_factor) {
  temperature_ = strip_units(IMP::internal::DEFAULT_TEMPERATURE);
  max_time_step_ = 2;
  current_time_ = 0;
  last_time_step_ = -1;
}

double Simulator::simulate(double time) {
  IMP_FUNCTION_LOG;
  set_is_optimizing_states(true);
  double ret = do_simulate(time);
  set_is_optimizing_states(false);
  return ret;
}

double Simulator::simulate_wave(double time, double max_time_step_factor,
                                double base) {
  IMP_FUNCTION_LOG;
  set_is_optimizing_states(true);
  double ret = do_simulate_wave(time, max_time_step_factor, base);
  set_is_optimizing_states(false);
  return ret;
}

double Simulator::do_simulate(double time) {
  IMP_FUNCTION_LOG;
  set_was_used(true);
  ParticleIndexes ps = get_simulation_particle_indexes();

  setup(ps);
  double target = current_time_ + time;
  boost::scoped_ptr<IMP::internal::BoostProgressDisplay> pgs;
  if (get_log_level() == PROGRESS) {
    pgs.reset(new IMP::internal::BoostProgressDisplay(time / max_time_step_));
  }
  while (current_time_ < target) {
    last_time_step_ = do_step(ps, max_time_step_);
    current_time_ += last_time_step_;
    update_states();
    if (get_log_level() == PROGRESS) {
      ++(*pgs);
    }
  }
  return Optimizer::get_scoring_function()->evaluate(false);
}

double Simulator::do_simulate_wave(double time, double max_time_step_factor,
                                   double base) {
  IMP_FUNCTION_LOG;
  set_was_used(true);
  ParticleIndexes ps = get_simulation_particle_indexes();

  setup(ps);
  double target = current_time_ + time;
  IMP_USAGE_CHECK(max_time_step_factor > 1.0,
                  "simulate wave time step factor must be >1.0");

  // build wave into cur_ts
  double wave_max_ts = max_time_step_factor * max_time_step_;
  std::vector<double> ts_seq;
  {
    int n_half = 2;  // subwave length
    bool max_reached = false;
    double raw_wave_time = 0.0;
    do {
      double cur_ts = max_time_step_;
      // go up
      for (int i = 0; i < n_half; i++) {
        ts_seq.push_back(cur_ts);
        raw_wave_time += cur_ts;
        cur_ts *= base;
        if (cur_ts > wave_max_ts) {
          max_reached = true;
          break;
        }
      }
      // go down
      for (int i = 0; i < n_half; i++) {
        cur_ts /= base;
        ts_seq.push_back(cur_ts);
        raw_wave_time += cur_ts;
        if (cur_ts < max_time_step_) {
          break;
        }
      }
      n_half++;
    } while (!max_reached && raw_wave_time < time);
    // adjust to fit into time precisely
    unsigned int n = (unsigned int)std::ceil(time / raw_wave_time);
    double wave_time = time / n;
    double adjust = wave_time / raw_wave_time;
    IMP_LOG(PROGRESS, "Wave time step seq: ");
    for (unsigned int i = 0; i < ts_seq.size(); i++) {
      ts_seq[i] *= adjust;
      IMP_LOG(PROGRESS, ts_seq[i] << ", ");
    }
    IMP_LOG(PROGRESS, std::endl);
  }

  unsigned int i = 0;
  unsigned int k = ts_seq.size();  // n waves of k frames
  int orig_nf_left = (int)(time / max_time_step_);
  while (current_time_ < target) {
    last_time_step_ = do_step(ps, ts_seq[i++ % k]);
    current_time_ += last_time_step_;
    // emulate state updating by frames for the origin max_time_step
    // (for periodic optimizers)
    int nf_left = (int)((target - current_time_) / max_time_step_);
    while (orig_nf_left >= nf_left) {
      IMP_LOG(PROGRESS, "Updating states: " << orig_nf_left << "," << nf_left
                                            << " target time " << target
                                            << " current time " << current_time_
                                            << std::endl);
      update_states();  // needs to move
      orig_nf_left--;
    }
  }
  IMP_LOG(PROGRESS, "Simulated for " << i << " actual frames with waves of "
                                     << k << " frames each" << std::endl);
  IMP_USAGE_CHECK(current_time_ >= target - 0.001 * max_time_step_,
                  "simulations did not advance to target time for some reason");
  return Optimizer::get_scoring_function()->evaluate(false);
}

ParticleIndexes Simulator::get_simulation_particle_indexes() const {
  IMP_OBJECT_LOG;
  ParticleIndexes ps;
  if (get_number_of_particles() == 0) {
    Model *m = get_model();
    ParticleIndexes pis = m->get_particle_indexes();
    for (ParticleIndexes::const_iterator it = pis.begin();
         it != pis.end(); ++it) {
      if (get_is_simulation_particle(*it)) {
        ps.push_back(*it);
      }
    }
  } else {
    ps = IMP::internal::get_index(
        ParticlesTemp(particles_begin(), particles_end()));
  }
  return ps;
}

ParticlesTemp Simulator::get_simulation_particles() const {
  ParticleIndexes p = get_simulation_particle_indexes();
  return IMP::internal::get_particle(get_model(), p);
}

double Simulator::do_optimize(unsigned int ns) {
  if (wave_factor_ >= 1.001) {
    return do_simulate_wave(ns * max_time_step_, wave_factor_);
  } else {
    return do_simulate(ns * max_time_step_);
  }
}

double Simulator::get_kt() const {
  return IMP::atom::get_kt(get_temperature());
}

IMP_LIST_IMPL(Simulator, Particle, particle, Particle *,
              Particles);

IMPATOM_END_NAMESPACE

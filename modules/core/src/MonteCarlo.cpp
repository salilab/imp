/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>
#include <IMP/core/model_io.h>

#include <IMP/random.h>
#include <IMP/Model.h>

#include <limits>
#include <cmath>
#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE

Mover::Mover() {}

IMP_LIST_IMPL(MonteCarlo, Mover, mover, Mover*, Movers,
              obj->set_optimizer(this),,);

MonteCarlo::MonteCarlo(): temp_(1),
                          stop_energy_(-std::numeric_limits<Float>::max()),
                          probability_(1),
                          num_local_steps_(50),
                          stat_forward_steps_taken_(0),
                          stat_upward_steps_taken_(0),
                          stat_num_failures_(0),
                          return_best_(false) {}


Float MonteCarlo::optimize(unsigned int max_steps)
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  double best_energy= std::numeric_limits<double>::max();
  std::string best_state;
  double prior_energy=std::numeric_limits<Float>::max();

  if (cg_) {
    IMP_CHECK_OBJECT(cg_.get());
    IMP_USAGE_CHECK(cg_->get_model() == get_model(),
               "The model used by the local optimizer does not match "\
              " that used by the montecarlo optimizer",
              InvalidStateException);
  }
  update_states();
  prior_energy =get_model()->evaluate(0);
  IMP_LOG(VERBOSE, "MC Initial energy is " << prior_energy << std::endl);
  ::boost::uniform_real<> rand(0,1);
  for (unsigned int i=0; i< max_steps; ++i) {
    //make it a parameter
    for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
      IMP_LOG(VERBOSE, "Moving using " << (*it)->get_name() << std::endl);
      IMP_CHECK_OBJECT(*it);
      (*it)->propose_move(probability_);
      IMP_LOG(VERBOSE, "end\n");
    }
    Float next_energy;
    if (cg_ && num_local_steps_!= 0) {
      IMP_LOG(VERBOSE,
              "MC Performing local optimization "<< std::flush);
      {
        IncreaseIndent ii;
        IMP_CHECK_OBJECT(cg_.get());

        // if incremental, turn off non-dirty particles
        if (get_model()->get_is_incremental()) {
          SaveOptimizeds si(ParticlesTemp(get_model()->particles_begin(),
                                          get_model()->particles_end()));
          for (Model::ParticleIterator it= get_model()->particles_begin();
               it != get_model()->particles_end(); ++it) {
            if (!(*it)->get_is_changed()) {
              for (Particle::FloatKeyIterator oit= (*it)->float_keys_begin();
                   oit != (*it)->float_keys_end(); ++oit) {
              (*it)->set_is_optimized(*oit, false);
              }
            }
          }
          next_energy =cg_->optimize(num_local_steps_);
        } else {
          next_energy =cg_->optimize(num_local_steps_);
        }
      }
      IMP_LOG(VERBOSE, next_energy << " done "<< std::endl);
    } else {
      next_energy =  get_model()->evaluate(0);
    }

    bool accept= (next_energy < prior_energy);
    if (!accept) {
      Float diff= next_energy- prior_energy;
      Float e= std::exp(-diff/temp_);
      Float r= rand(random_number_generator);
      IMP_LOG(VERBOSE, diff << " " << temp_ << " " << e << " " << r
              << std::endl);
      if (e > r) {
        accept=true;
        ++stat_upward_steps_taken_;
      }
    }
    IMP_LOG(TERSE,  "MC Prior energy is " << prior_energy
            << " and next is " << next_energy << " ");
    if (accept) {
      IMP_LOG(TERSE,  " accept" << std::endl);
      for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
        (*it)->accept_move();
      }

      if (return_best_ && next_energy < best_energy) {
        best_energy= next_energy;
        std::ostringstream oss;
        write(get_model(), oss);
        best_state= oss.str();
      }
    } else {
      IMP_LOG(TERSE,  " reject" << std::endl);
      for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
        (*it)->reject_move();
      }
    }

    if (accept) {
      ++stat_forward_steps_taken_;
      prior_energy= next_energy;
      update_states();
    } else {
      ++stat_num_failures_;
    }
    if (prior_energy < stop_energy_) break;
  }
  IMP_LOG(TERSE, "MC Final energy is " << prior_energy << std::endl);
  if (return_best_) {
    std::istringstream iss(best_state);
    SetLogState ll(VERBOSE);
    read(iss, get_model());
    IMP_LOG(TERSE, "MC Returning energy " << best_energy << std::endl);
    IMP_IF_CHECK(USAGE) {
      double e= get_model()->evaluate(false);
      IMP_LOG(TERSE, "MC Got " << e << std::endl);
      IMP_INTERNAL_CHECK(std::abs(best_energy - e)
                 < .1* (best_energy +e), "Energies do not match "
                 << best_energy << " vs " << e << std::endl);
    }
    return best_energy;
  } else {
    return get_model()->evaluate(false); //force coordinate update
  }
}


void MonteCarlo::set_local_optimizer(Optimizer* cg)
{
  cg_= cg;
  cg_->set_model(get_model());
}

void MonteCarlo::show(std::ostream &out) const
{
  out << "MonteCarlo +" << stat_forward_steps_taken_
      << " -" << stat_num_failures_ << std::endl;
}

IMPCORE_END_NAMESPACE

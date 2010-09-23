/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>

#include <IMP/random.h>
#include <IMP/Model.h>
#include <IMP/ConfigurationSet.h>

#include <limits>
#include <cmath>
#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE

Mover::Mover(std::string name):Object(name) {}

IMP_LIST_IMPL(MonteCarlo, Mover, mover, Mover*, Movers,
              {obj->set_optimizer(this);
                obj->set_was_used(true);
              },{},{});

MonteCarlo::MonteCarlo(Model *m): Optimizer(m, "MonteCarlo"),
                                  temp_(1),
                                  probability_(1),
                                  num_local_steps_(50),
                                  stat_forward_steps_taken_(0),
                                  stat_upward_steps_taken_(0),
                                  stat_num_failures_(0),
                                  return_best_(true) {}


Float MonteCarlo::optimize(unsigned int max_steps)
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (get_number_of_movers() ==0) {
    IMP_THROW("Running MonteCarlo without providing any"
              << " movers isn't very useful.",
              ValueException);
  }
  double best_energy= std::numeric_limits<double>::max();
  IMP::internal::OwnerPointer<Configuration> best_state
    = new Configuration(get_model());

  int failures=0;
  if (cg_) {
    cg_->set_score_threshold(get_score_threshold());
    IMP_CHECK_OBJECT(cg_.get());
    IMP_USAGE_CHECK(cg_->get_model() == get_model(),
               "The model used by the local optimizer does not match "\
              " that used by the montecarlo optimizer");
  }
  update_states();
  double prior_energy =get_model()->evaluate(false);
  if (prior_energy < get_score_threshold()) return prior_energy;

  IMP_LOG(TERSE, "MC Initial energy is " << prior_energy << std::endl);

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
      IMP_LOG(TERSE,
              "MC Performing local optimization from "
              << get_model()->evaluate(false) << std::endl);
      {
        IncreaseIndent ii;
        IMP_CHECK_OBJECT(cg_.get());

        // if incremental, turn off non-dirty particles
        try {
          if (get_model()->get_is_incremental()) {
            bool has_changed=false;
            get_model()->update();
            SaveOptimizeds si(ParticlesTemp(get_model()->particles_begin(),
                                            get_model()->particles_end()));
            for (Model::ParticleIterator it= get_model()->particles_begin();
                 it != get_model()->particles_end(); ++it) {
              if (!(*it)->get_is_changed()) {
                for (Particle::FloatKeyIterator oit= (*it)->float_keys_begin();
                     oit != (*it)->float_keys_end(); ++oit) {
                  (*it)->set_is_optimized(*oit, false);
                }
              } else {
                has_changed=true;
                IMP_LOG(VERBOSE, "Particle " << (*it)->get_name()
                        << " was changed " << **it << std::endl);
              }
            }
            if (has_changed) {
              next_energy =cg_->optimize(num_local_steps_);
            } else {
              IMP_LOG(TERSE, "empty move" << std::endl);
              next_energy=prior_energy;
            }
          } else {
            next_energy =cg_->optimize(num_local_steps_);
            IMP_IF_CHECK(USAGE) {
              double me= get_model()->evaluate(false);
              IMP_USAGE_CHECK((next_energy-me) < .01*(next_energy+me),
                              "Energies don't match after local opt. "
                              << "Got " << me << " but computed "
                              << next_energy);
            }
          }
        } catch (const ModelException &e) {
          // make sure the move is rejected if the model gets in
          // an invalid state
          ++failures;
          next_energy= std::numeric_limits<double>::infinity();
        }
      }
      IMP_LOG(TERSE, "To energy " << next_energy << " equals "
              << get_model()->evaluate(false)
              << " done "<< std::endl);
    } else {
      next_energy =  get_model()->evaluate(false);
    }
    bool accept=false;
    if  (next_energy < prior_energy) {
      accept=true;
    } else {
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
            << " and next is " << next_energy << " "
            << "(" << get_score_threshold() << ") ");
    if (accept) {
      IMP_LOG(TERSE,  " accept" << std::endl);
      for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
        (*it)->accept_move();
      }
      ++stat_forward_steps_taken_;
      prior_energy= next_energy;
      if (return_best_) {
        best_energy= next_energy;
        IMP_LOG(TERSE, "Saving state with energy " << best_energy << std::endl);
        /*for (Model::ParticleIterator it = get_model()->particles_begin();
             it != get_model()->particles_end(); ++it) {
          if (XYZ::particle_is_instance(*it)) std::cout
          << XYZ(*it) << std::endl;
          }*/
        best_state= new Configuration(get_model());
        best_state->load_configuration();
      }
      if (next_energy < get_score_threshold()) break;
      update_states();
    } else {
      IMP_LOG(TERSE,  " reject" << std::endl);
      for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
        (*it)->reject_move();
      }
      ++stat_num_failures_;
    }
  }


  IMP_LOG(TERSE, "MC Final energy is " << prior_energy
          << " after " << failures << " failures" << std::endl);
  if (return_best_) {
    /*std::cout << "model was ";
    for (Model::ParticleIterator it = get_model()->particles_begin();
         it != get_model()->particles_end(); ++it) {
      if (XYZ::particle_is_instance(*it)) std::cout << XYZ(*it) << std::endl;
      }*/
    best_state->load_configuration();
    /*std::cout << "model is ";
    for (Model::ParticleIterator it = get_model()->particles_begin();
         it != get_model()->particles_end(); ++it) {
      if (XYZ::particle_is_instance(*it)) std::cout << XYZ(*it) << std::endl;
      }*/
    IMP_LOG(TERSE, "MC Returning energy " << best_energy << std::endl);
    IMP_IF_CHECK(USAGE) {
      IMP_CHECK_CODE(double e=) get_model()->evaluate(false);
      IMP_LOG(TERSE, "MC Got " << e << std::endl);
      IMP_INTERNAL_CHECK(std::abs(best_energy - e)
                 < .01+.1* (best_energy +e), "Energies do not match "
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

void MonteCarlo::do_show(std::ostream &out) const
{
  out << "forward steps " << stat_forward_steps_taken_
      << " -" << stat_num_failures_ << std::endl;
}

IMPCORE_END_NAMESPACE

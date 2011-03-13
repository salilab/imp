/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>

#include <IMP/random.h>
#include <IMP/Model.h>
#include <IMP/ConfigurationSet.h>

#include <limits>
#include <cmath>
#include <boost/scoped_ptr.hpp>

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
                                  return_best_(true),
                                  basin_hopping_(false),
                                  rand_(0,1){}

namespace {
  double do_local(Optimizer *opt, int ns, bool basin_hopping) {
    Pointer<Configuration> cs;
    if (basin_hopping) {
      cs= new Configuration(opt->get_model());
    }
    double ne =opt->optimize(ns);
    if (basin_hopping) {
      cs->load_configuration();
    }
    return ne;
  }
}

 bool MonteCarlo::get_accept(double prior_energy, double next_energy){
   if  (next_energy < prior_energy) {
     return true;
   } else {
     Float diff= next_energy- prior_energy;
     Float e= std::exp(-diff/temp_);
     Float r= rand_(random_number_generator);
     IMP_LOG(VERBOSE, diff << " " << temp_ << " " << e << " " << r
             << std::endl);
     if (e > r) {
       ++stat_upward_steps_taken_;
       return true;
     }
   }
   return false;
}

double MonteCarlo::do_step(double prior_energy, double &best_energy,
                 IMP::internal::OwnerPointer<Configuration> &best_state,
                           int &failures) {
  for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
    IMP_LOG(VERBOSE, "Moving using " << (*it)->get_name() << std::endl);
    IMP_CHECK_OBJECT(*it);
    (*it)->propose_move(probability_);
    IMP_LOG(VERBOSE, "end\n");
  }
  double next_energy;
  try {
    if (cg_ && num_local_steps_!= 0) {
      IMP_LOG(TERSE,
              "MC Performing local optimization from "
              << evaluate(false) << std::endl);
      CreateLogContext clc("mc local optimization");
      boost::scoped_ptr<SaveOptimizeds> so;
      {
        IMP_CHECK_OBJECT(cg_.get());
        // if incremental, turn off non-dirty particles
        if (get_model()->get_is_incremental()) {
          bool has_changed=false;
          get_model()->update();
     so.reset(new SaveOptimizeds(ParticlesTemp(get_model()->particles_begin(),
                                              get_model()->particles_end())));
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
          } else {
            IMP_LOG(TERSE, "empty move" << std::endl);
            return prior_energy;
          }
        }
      }
      next_energy =do_local(cg_, num_local_steps_, basin_hopping_);
      IMP_LOG(TERSE, "To energy " << next_energy << " equals "
              << evaluate(false)
              << " done "<< std::endl);
    } else {
      next_energy =  evaluate(false);
    }
  } catch (const ModelException &e) {
    // make sure the move is rejected if the model gets in
    // an invalid state
    ++failures;
    next_energy= std::numeric_limits<double>::infinity();
  }
  IMP_LOG(TERSE,  "MC Prior energy is " << prior_energy
          << " and next is " << next_energy << " "
          << "(" << get_score_threshold() << ") ");
  if (get_accept(prior_energy, next_energy)) {
    IMP_LOG(TERSE,  " accept" << std::endl);
    ++stat_forward_steps_taken_;
    prior_energy= next_energy;
    if (return_best_ && next_energy < best_energy) {
      best_energy= next_energy;
      IMP_LOG(TERSE, "Saving state with energy " << best_energy << std::endl);
      //std::cout << "best energy is first " << best_energy << std::endl;
      best_state= new Configuration(get_model());
      //best_state->load_configuration();
      /*IMP_IF_CHECK(USAGE) {
        std::cout << "best energy is " << best_energy << std::endl;
        best_state->load_configuration();
        std::cout << "best energy is now " << best_energy << std::endl;
        }*/
    }
    update_states();
  } else {
    IMP_LOG(TERSE,  " reject" << std::endl);
    for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
      (*it)->reset_move();
    }
    ++stat_num_failures_;
  }
  return prior_energy;
}


Float MonteCarlo::do_optimize(unsigned int max_steps)
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (get_number_of_movers() ==0) {
    IMP_THROW("Running MonteCarlo without providing any"
              << " movers isn't very useful.",
              ValueException);
  }
  IMP::internal::OwnerPointer<Configuration> best_state
    = new Configuration(get_model());

  stat_forward_steps_taken_ = 0;
  stat_num_failures_ = 0;
  int failures=0;
  if (cg_) {
    //cg_->set_score_threshold(get_score_threshold());
    IMP_CHECK_OBJECT(cg_.get());
    IMP_USAGE_CHECK(cg_->get_model() == get_model(),
                    "The model used by the local optimizer does not match "\
                    " that used by the montecarlo optimizer");
  }
  update_states();
  double prior_energy =evaluate(false);
  double best_energy= prior_energy;
  //if (prior_energy < get_score_threshold()) return prior_energy;

  IMP_LOG(TERSE, "MC Initial energy is " << prior_energy << std::endl);

  for (unsigned int i=0; i< max_steps; ++i) {
    if (get_stop_on_good_score() && get_model()->get_has_good_score()) {
      return prior_energy;
    }
    do_step(prior_energy, best_energy,
            best_state, failures);
  }


  IMP_LOG(TERSE, "MC Final energy is " << prior_energy
          << " after " << failures << " failures" << std::endl);
  if (return_best_) {
    //std::cout << "Final score is " << get_model()->evaluate(false)
    //<< std::endl;
    best_state->load_configuration();
    IMP_LOG(TERSE, "MC Returning energy " << best_energy << std::endl);
    IMP_IF_CHECK(USAGE) {
      IMP_CHECK_CODE(double e= evaluate(false));
      IMP_LOG(TERSE, "MC Got " << e << std::endl);
      IMP_INTERNAL_CHECK(std::abs(best_energy - e)
                         < .01+.1* std::abs(best_energy +e),
                         "Energies do not match "
                         << best_energy << " vs " << e << std::endl);
    }
    return best_energy;
  } else {
    double ret= evaluate(false); //force coordinate update
    IMP_INTERNAL_CHECK(ret < std::numeric_limits<double>::max(),
                       "Don't return rejected conformation");
    return ret;
  }
}


void MonteCarlo::set_use_basin_hopping(bool tf) {
  IMP_USAGE_CHECK(cg_, "A local optimizer must be set for "
                  << "basin hopping to be used.");
  basin_hopping_=tf;
}

void MonteCarlo::set_local_optimizer(Optimizer* cg)
{
  cg_= cg;
  cg_->set_model(get_model());
}

void MonteCarlo::do_show(std::ostream &out) const
{
  out << "forward steps taken after last call " << stat_forward_steps_taken_
      << " -" << stat_num_failures_ << std::endl;
}

IMPCORE_END_NAMESPACE

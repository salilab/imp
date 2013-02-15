/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>

#include <IMP/random.h>
#include <IMP/Model.h>
#include <IMP/ConfigurationSet.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/dependency_graph.h>

#include <limits>
#include <cmath>
#include <boost/scoped_ptr.hpp>

IMPCORE_BEGIN_NAMESPACE

Mover::Mover(Model *m, std::string name):ModelObject(m, name) {}

IMP_LIST_IMPL(MonteCarlo, Mover, mover, Mover*, Movers);

MonteCarlo::MonteCarlo(Model *m): Optimizer(m, "MonteCarlo%1%"),
                                  temp_(1),
                      max_difference_(std::numeric_limits<double>::max()),
                                  probability_(1),
                                  stat_forward_steps_taken_(0),
                                  stat_upward_steps_taken_(0),
                                  stat_num_failures_(0),
                                  return_best_(true),
                                  rand_(0,1){}

bool MonteCarlo::do_accept_or_reject_move(double score, double last) {
  bool ok=false;
  if  (score < last) {
    ok=true;
    if (score < best_energy_ && return_best_) {
      best_= new Configuration(get_model());
      best_energy_=score;
    }
  } else {
    double diff= score- last;
    double e= std::exp(-diff/temp_);
    double r= rand_(random_number_generator);
    IMP_LOG_VERBOSE( diff << " " << temp_ << " " << e << " " << r
            << std::endl);
    if (e > r) {
      ++stat_upward_steps_taken_;
      ok=true;
    } else {
      ok=false;
    }
  }
  if (ok) {
    IMP_LOG_TERSE( "Accept: " << score
            << " previous score was " << last << std::endl);
    ++stat_forward_steps_taken_;
    last_energy_=score;
    update_states();
    return true;
  } else {
    IMP_LOG_TERSE( "Reject: " << score
            << " current score stays " << last << std::endl);
    for (int i= get_number_of_movers()-1; i>=0; --i) {
      get_mover(i)->reset_move();
    }
    ++stat_num_failures_;
    if (isf_) {
      isf_->reset_moved_particles();
    }
    return false;
  }
}

ParticlesTemp MonteCarlo::do_move(double probability) {
  ParticlesTemp ret;
  for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
    IMP_LOG_VERBOSE( "Moving using " << (*it)->get_name() << std::endl);
    IMP_CHECK_OBJECT(*it);
    {
      //IMP_LOG_CONTEXT("Mover " << (*it)->get_name());
      ParticlesTemp cur=(*it)->propose_move(probability);
      ret.insert(ret.end(), cur.begin(), cur.end());
    }
    IMP_LOG_VERBOSE( "end\n");
  }
  return ret;
}

void MonteCarlo::do_step() {
  ParticlesTemp moved=do_move(probability_);
  double energy= do_evaluate(moved);
  do_accept_or_reject_move(energy);
}

double MonteCarlo::do_optimize(unsigned int max_steps) {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (get_number_of_movers() ==0) {
    IMP_THROW("Running MonteCarlo without providing any"
              << " movers isn't very useful.",
              ValueException);
  }
  ParticlesTemp movable;
  for (unsigned int i=0; i< get_number_of_movers(); ++i) {
    movable+= get_mover(i)->get_output_particles();
  }
  // provide a way of feeding in this value
  last_energy_ =do_evaluate(movable);
  if (return_best_) {
    best_= new Configuration(get_model());
    best_energy_= last_energy_;
  }
  stat_forward_steps_taken_ = 0;
  stat_num_failures_ = 0;
  update_states();

  IMP_LOG_TERSE( "MC Initial energy is " << last_energy_ << std::endl);

  for (unsigned int i=0; i< max_steps; ++i) {
    if (get_stop_on_good_score()
        && get_scoring_function()->get_had_good_score()) {
      break;
    }
    do_step();
    if (best_energy_ < get_score_threshold()) break;
  }

  IMP_LOG_TERSE( "MC Final energy is " << last_energy_  << std::endl);
  if (return_best_) {
    //std::cout << "Final score is " << get_model()->evaluate(false)
    //<< std::endl;
    best_->swap_configuration();
    IMP_LOG_TERSE( "MC Returning energy " << best_energy_ << std::endl);
    IMP_IF_CHECK(base::USAGE) {
      ParticlesTemp movable;
      for (unsigned int i=0; i< get_number_of_movers(); ++i) {
        movable+= get_mover(i)->get_output_particles();
      }
      IMP_LOG_TERSE( "MC Got " << do_evaluate(movable) << std::endl);
      /*IMP_INTERNAL_CHECK((e >= std::numeric_limits<double>::max()
                          && best_energy_ >= std::numeric_limits<double>::max())
                         || std::abs(best_energy_ - e)
                         < .01+.1* std::abs(best_energy_ +e),
                         "Energies do not match "
                         << best_energy_ << " vs " << e << std::endl);*/
    }
    return get_scoring_function()->evaluate(false);
  } else {
    return last_energy_;
  }
}
void
MonteCarlo::set_incremental_scoring_function(IncrementalScoringFunction *isf) {
  isf_=isf;
  Optimizer::set_scoring_function(isf);
}
void MonteCarlo::do_show(std::ostream &) const {
}


MonteCarloWithLocalOptimization::MonteCarloWithLocalOptimization(Optimizer *opt,
                                                           unsigned int steps):
  MonteCarlo(opt->get_model()), opt_(opt), num_local_(steps) {}


void MonteCarloWithLocalOptimization::do_step() {
  ParticlesTemp moved=do_move(get_move_probability());
  IMP_LOG_TERSE(
          "MC Performing local optimization from "
          << do_evaluate(moved) << std::endl);
  // non-Mover parts of the model can be moved by the local optimizer
  // make sure they are cleaned up
  OwnerPointer<Configuration> cs= new Configuration(get_model());
  double ne =opt_->optimize(num_local_);
  if (!do_accept_or_reject_move(ne)) {
    cs->swap_configuration();
  }
}

void MonteCarloWithLocalOptimization::do_show(std::ostream &) const {
}



MonteCarloWithBasinHopping::MonteCarloWithBasinHopping(Optimizer *opt,
                                                       unsigned int steps):
  MonteCarloWithLocalOptimization(opt, steps) {}


void MonteCarloWithBasinHopping::do_step() {
  ParticlesTemp moved=do_move(get_move_probability());
  IMP_LOG_TERSE(
          "MC Performing local optimization from "
          << do_evaluate(moved) << std::endl);
  Pointer<Configuration> cs= new Configuration(get_model());
  double ne =get_local_optimizer()->optimize(get_number_of_steps());
  cs->swap_configuration();
  do_accept_or_reject_move(ne);
}

void MonteCarloWithBasinHopping::do_show(std::ostream &) const {
}


IMPCORE_END_NAMESPACE

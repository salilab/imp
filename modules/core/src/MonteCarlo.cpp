/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/MonteCarlo.h>

#include <IMP/random.h>
#include <IMP/Model.h>

#include <limits>
#include <cmath>
#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE

Mover::Mover() {}

Mover::~Mover(){}


IMP_LIST_IMPL(MonteCarlo, Mover, mover, Mover*,
              obj->set_optimizer(this),,);

MonteCarlo::MonteCarlo(): temp_(1),
                          prior_energy_(std::numeric_limits<Float>::max()),
                          stop_energy_(-std::numeric_limits<Float>::max()),
                          probability_(1),
                          num_local_steps_(50),
                          stat_forward_steps_taken_(0),
                          stat_upward_steps_taken_(0),
                          stat_num_failures_(0){}

MonteCarlo::~MonteCarlo()
{
}

Float MonteCarlo::optimize(unsigned int max_steps)
{
  IMP_CHECK_OBJECT(this);
  if (cg_) {
    IMP_CHECK_OBJECT(cg_.get());
    IMP_check(cg_->get_model() == get_model(),
               "The model used by the local optimizer does not match "\
              " that used by the montecarlo optimizer",
              InvalidStateException);
  }
  update_states();
  prior_energy_ =get_model()->evaluate(0);
  IMP_LOG(VERBOSE, "MC Initial energy is " << prior_energy_ << std::endl);
  ::boost::uniform_real<> rand(0,1);
  for (unsigned int i=0; i< max_steps; ++i) {
    //make it a parameter
    for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
      IMP_LOG(VERBOSE, "MC Trying move " << **it << std::endl);
      IMP_CHECK_OBJECT(*it);
      (*it)->propose_move(probability_);
    }
    Float next_energy;
    if (cg_ && num_local_steps_!= 0) {
      IMP_LOG(VERBOSE,
              "MC Performing local optimization "<< std::flush);
      IMP_CHECK_OBJECT(cg_.get());
      next_energy =cg_->optimize(num_local_steps_);
      IMP_LOG(VERBOSE, next_energy << " done "<< std::endl);
    } else {
      next_energy =  get_model()->evaluate(0);
    }

    bool accept= (next_energy < prior_energy_);
    if (!accept) {
      Float diff= next_energy- prior_energy_;
      Float e= std::exp(-diff/temp_);
      Float r= rand(random_number_generator);
      IMP_LOG(VERBOSE, diff << " " << temp_ << " " << e << " " << r
              << std::endl);
      if (e > r) {
        accept=true;
        ++stat_upward_steps_taken_;
      }
    }
    IMP_LOG(VERBOSE,  "MC Prior energy is " << prior_energy_
            << " and next is " << next_energy << " ");
    if (accept) {
      IMP_LOG(VERBOSE,  " accept" << std::endl);
      for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
        (*it)->accept_move();
      }
    } else {
      IMP_LOG(VERBOSE,  " reject" << std::endl);
      for (MoverIterator it = movers_begin(); it != movers_end(); ++it) {
        (*it)->reject_move();
      }
    }

    if (accept) {
      ++stat_forward_steps_taken_;
      prior_energy_= next_energy;
      update_states();
    } else {
      ++stat_num_failures_;
    }
    if (prior_energy_ < stop_energy_) break;
  }
  IMP_LOG(VERBOSE, "MC Final energy is " << prior_energy_ << std::endl);
  return prior_energy_;
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

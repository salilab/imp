/**
 *  \file MonteCarlo.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_H
#define IMPCORE_MONTE_CARLO_H

#include "core_config.h"
#include "Mover.h"
#include "core_macros.h"
#include <IMP/Optimizer.h>
#include <IMP/container_macros.h>
#include <IMP/Configuration.h>

#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE


//! A Monte Carlo optimizer.
/** The optimizer uses a set of Mover objects to propose steps. Currently
    each Mover is called at each Monte Carlo iteration. This may change in
    the future. The movers propose some modification, which is then
    accepted or rejected based on the Metropolis criteria. Optionally, a
    number of local optimization steps are taken before the MonteCarlo step
    is accepted or rejected.

    By default, the lowest score state encountered is returned.

    \see Mover
 */
class IMPCOREEXPORT MonteCarlo: public Optimizer
{
public:
  MonteCarlo(Model *m=NULL);

  IMP_OPTIMIZER(MonteCarlo);
 public:
  /** By default, the optimizer returns the lowest score state
      found so far. If, instead, you wish to return the last accepted
      state, set return best to false.
  */
  void set_return_best(bool tf) {
    return_best_=tf;
  }

  /** \name kT
      The kT value has to be on the same scale as the differences
      in energy between good and bad states (and so the default is
      likely to not be a good choice).
      @{
  */
  void set_kt(Float t) {
    IMP_INTERNAL_CHECK(t>0, "Temperature must be positive");
    temp_=t;
  }
  Float get_kt() const {
    return temp_;
  }
  /** @} */
  /** Return the energy of last accepted state.
   */
  double get_last_accepted_energy() const {
    return last_energy_;
  }

  /** If return best is on, you can get the best energy
      found so far.*/
  double get_best_accepted_energy() const {
    IMP_USAGE_CHECK(return_best_, "Getting the best energy"
                    << " requires return best being on.");
    return best_energy_;
  }

  //! Set the probability of each move being made
  /** Make this low if the space is rough and there are many particles.
      The movers should make each individual move with this probability.
      That is, a NormalMover with 100 particles will move each particle
      with probability p.
   */
  void set_move_probability(Float p) {
    IMP_USAGE_CHECK(p > 0 && p <= 1, "Not a valid probability");
    probability_=p;
  }
  double get_move_probability() const {
    return probability_;
  }
  /** \name Statistics
      @{
   */
  //! Return how many times the optimizer has succeeded in taking a step
  unsigned int get_number_of_forward_steps() const {
    return stat_forward_steps_taken_;
  }
  //! Return how many times the optimizer has stepped to higher energy
  unsigned int get_number_of_upward_steps() const {
    return stat_upward_steps_taken_;
  }
  /** @} */

  /** Computations can be acceletating by throwing out
      the tails of the distribution of accepted moves. To
      do this, specific a maximum acceptable difference
      between the before and after scores.
  */
  void set_maximum_difference(double d) {
    max_difference_=d;
  }

  double get_maximum_difference() const {
    return max_difference_;
  }
  /** @name Movers

       The following methods are used to manipulate the list of Movers.
       Each mover is called at each optimization step, giving it a chance
       to change the current configuration.
       @{
  */
  IMP_LIST_ACTION(public, Mover, Movers, mover, movers, Mover*, Movers,
                  {obj->set_optimizer(this);
                    obj->set_was_used(true);
                  },{},{obj->set_optimizer(NULL);});
  /** @} */
 protected:
  /** Note that if return best is true, this will save the current
      state of the model. Also, if the move is accepted, the
      optimizer states will be updated.
  */
  bool do_accept_or_reject_move(double score);
  void do_move(double probability);
  //! a class that inherits from this should override this method
  virtual void do_step();
  //! Get the current energy
  /** By default it just calls Optimizer::evaluate() if there is
      no maximum allowed difference or Optimizer::evaluate_if_below()
      if there is. Classes which override this method should be
      similarly aware for efficiency.
   */
  virtual double do_evaluate() const {
    if (get_maximum_difference()
        < std::numeric_limits<double>::max()) {
      return evaluate_if_below(false, last_energy_+max_difference_);
    } else {
      return evaluate(false);
    }
  }
private:
  double temp_;
  double last_energy_;
  double best_energy_;
  double max_difference_;
  Float probability_;
  unsigned int stat_forward_steps_taken_;
  unsigned int stat_upward_steps_taken_;
  unsigned int stat_num_failures_;
  bool return_best_;
  IMP::internal::OwnerPointer<Configuration> best_;
  ::boost::uniform_real<> rand_;
};



//! This variant of Monte Carlo that relaxes after each move
class IMPCOREEXPORT MonteCarloWithLocalOptimization: public MonteCarlo
{
  IMP::internal::OwnerPointer<Optimizer> opt_;
  unsigned int num_local_;
public:
  MonteCarloWithLocalOptimization(Optimizer *opt,
                               unsigned int steps);

  unsigned int get_number_of_steps() const {
    return num_local_;
  }

  Optimizer* get_local_optimizer() const {
    return opt_;
  }

  IMP_MONTE_CARLO(MonteCarloWithLocalOptimization);
};

//! This variant of Monte Carlo uses basis hopping
/** Basin hopping is where, after a move, a local optimizer is used to relax
    the model before the energy computation. However, the pre-relaxation state
    of the model is used as the starting point for the next step. The idea
    is that models are accepted or rejected based on the score of the nearest
    local minima, but they can still climb the barriers in between as the model
    is not reset to the minima after each step.
 */
class IMPCOREEXPORT MonteCarloWithBasinHopping:
public MonteCarloWithLocalOptimization
{
public:
  MonteCarloWithBasinHopping(Optimizer *opt, unsigned int ns);

  IMP_MONTE_CARLO(MonteCarloWithBasinHopping);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MONTE_CARLO_H */

/**
 *  \file IMP/core/MonteCarlo.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_H
#define IMPCORE_MONTE_CARLO_H

#include <IMP/core/core_config.h>
#include "Mover.h"
#include "monte_carlo_macros.h"
#include "IncrementalScoringFunction.h"
#include <IMP/Optimizer.h>
#include <IMP/optimizer_macros.h>
#include <IMP/container_macros.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/Configuration.h>

#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE


//! A Monte Carlo optimizer.
/** The optimizer uses a set of Mover objects to propose steps. Currently
    each Mover is called at each Monte Carlo iteration. If you only want to
    use one mover at a time, use a SerialMover.
    The movers propose some modification, which is then
    accepted or rejected based on the Metropolis criterion. Optionally, a
    number of local optimization steps are taken before the MonteCarlo step
    is accepted or rejected.

    By default, the lowest score state encountered is returned.

    \see Mover
 */
class IMPCOREEXPORT MonteCarlo: public Optimizer
{
public:
  MonteCarlo(Model *m=nullptr);

 protected:
  virtual Float do_optimize(unsigned int max_steps);
  IMP_OBJECT_METHODS(MonteCarlo)
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
  IMP_LIST_ACTION(public, Mover, Movers, mover, movers,
                  MonteCarloMover*,
                  MonteCarloMovers,
                  {},{},{});
  /** @} */


  /** \name Incremental
      Efficient evaluation of non-bonded list based restraints is
      a bit tricky with incremental evaluation.
      @{
  */
  /** Set whether to use incremental evaluate or evaluate all restraints
      each time. This cannot be changed during optimization.
  */
  void set_incremental_scoring_function(IncrementalScoringFunction *isf);
  bool get_use_incremental_scoring_function() const {
    return isf_;
  }
  IncrementalScoringFunction* get_incremental_scoring_function() const {
    return isf_;
  }
  /** @} */
 protected:
  /** Note that if return best is true, this will save the current
      state of the model. Also, if the move is accepted, the
      optimizer states will be updated.
  */
  bool do_accept_or_reject_move(double score, double last,
                                double proposal_ratio);
  bool do_accept_or_reject_move(double score, double proposal_ratio)
  {
    return do_accept_or_reject_move(score, get_last_accepted_energy(),
                                    proposal_ratio);
  }

  MonteCarloMoverResult do_move();
  //! a class that inherits from this should override this method
  virtual void do_step();
  //! Get the current energy
  /** By default it just calls
      Optimizer::get_scoring_function()->evaluate(false). However,
      if an incremental scoring function is used, the list of moved
      particles will be used to evaluate the score more efficiently.
      Also, if there is a maximum allowed difference in scores
      Optimizer::get_scoring_function()->evaluate_if_below()
      will be called instead, allowing more efficient evaluation.
      Classes which override this method should be similarly aware for
      efficiency.

      The list of moved particles is passed.
   */
  virtual double do_evaluate(const ParticleIndexes &moved) const {
    IMP_UNUSED(moved);
    if (isf_ ) {
      isf_->set_moved_particles(moved);
    }
    if (get_maximum_difference() < NO_MAX) {
      return get_scoring_function()
        ->evaluate_if_below(false, last_energy_+max_difference_);
    } else {
      return get_scoring_function()
        ->evaluate(false);
    }
  }
private:
  double temp_;
  double last_energy_;
  double best_energy_;
  double max_difference_;
  unsigned int stat_forward_steps_taken_;
  unsigned int stat_upward_steps_taken_;
  unsigned int stat_num_failures_;
  bool return_best_;
  IMP::OwnerPointer<Configuration> best_;
  ::boost::uniform_real<> rand_;

  Pointer<IncrementalScoringFunction> isf_;
};



//! This variant of Monte Carlo that relaxes after each move
class IMPCOREEXPORT MonteCarloWithLocalOptimization: public MonteCarlo
{
  IMP::OwnerPointer<Optimizer> opt_;
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

 protected:
  virtual void do_step() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MonteCarloWithLocalOptimization);
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

 protected:
  virtual void do_step() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MonteCarloWithBasinHopping);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MONTE_CARLO_H */

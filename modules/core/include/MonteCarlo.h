/**
 *  \file IMP/core/MonteCarlo.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_H
#define IMPCORE_MONTE_CARLO_H

#include <IMP/core/core_config.h>
#include "MonteCarloMover.h"
#include "IncrementalScoringFunction.h"
#include <IMP/Optimizer.h>
#include <IMP/container_macros.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/Configuration.h>

#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE

/** Allow code to test for the changes in MC interface.*/
#define IMP_CORE_HAS_MONTE_CARLO_MOVER 1

//! A Monte Carlo optimizer.
/** The optimizer uses a set of Mover objects to propose steps.  At
    each sampling iteration, all Movers added to MonteCarlo are called to
    generate a new proposed configuration.

    The movers propose some modification, which is then accepted or
    rejected based on the Metropolis criterion. Optionally, a number
    of local optimization steps are taken before the MonteCarlo step
    is accepted or rejected.

    If you want to sequentially call one mover at every iteration, wrap
    all movers into a SerialMover first, and then add the SerialMover to
    MonteCarlo.

    By default, the lowest score state encountered is returned.

    \see Mover
 */
class IMPCOREEXPORT MonteCarlo : public Optimizer {
 public:
  MonteCarlo(Model *m);

 protected:
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MonteCarlo)
 public:
  /** By default, the optimizer returns the lowest scoring state
      found so far. If, instead, you wish to return the last accepted
      state, set return best to false.
  */
  void set_return_best(bool tf) { return_best_ = tf; }

  //! If set true (default false), only rescore moved particles
  /** By default, on each move the score of the entire system is
      calculated. If it is guaranteed that only Movers and ScoreStates
      move the system, then the score can potentially be calculated
      more quickly by caching the scores on parts of the system that
      don't move. This is still experimental.

      \note Some MonteCarlo subclasses do local optimization after each
            move, which can move more particles than the Movers touched.
            In this case the guarantee does not hold and this optimization
            should probably not be used.
   */
  void set_score_moved(bool mv) { score_moved_ = mv; }

  /** \name kT
      The kT value has to be on the same scale as the differences
      in energy between good and bad states (and so the default is
      likely to not be a good choice).
      @{
  */
  void set_kt(Float t) {
    IMP_INTERNAL_CHECK(t > 0, "Temperature must be positive");
    temp_ = t;
  }
  Float get_kt() const { return temp_; }
  /** @} */
  //! Return the energy of the last accepted state.
  double get_last_accepted_energy() const { return last_energy_; }

  //! If return best is on, returns the best energy found so far.
  double get_best_accepted_energy() const {
    IMP_USAGE_CHECK(return_best_, "Getting the best energy"
                                      << " requires return best being on.");
    return best_energy_;
  }
  /** \name Statistics
      @{
   */
  //! Return how many times the optimizer has stepped to lower score
  unsigned int get_number_of_downward_steps() const {
    return stat_downward_steps_taken_;
  }
  //! Return how many times the optimizer has stepped to higher score
  unsigned int get_number_of_upward_steps() const {
    return stat_upward_steps_taken_;
  }
  //! Get number of proposed moves
  unsigned int get_number_of_proposed_steps() const {
    return stat_downward_steps_taken_ + stat_upward_steps_taken_ +
           stat_num_failures_;
  }
  //! Get number of accepted moves
  unsigned int get_number_of_accepted_steps() const {
    return stat_downward_steps_taken_ + stat_upward_steps_taken_;
  }
  void reset_statistics() {
    stat_downward_steps_taken_ = 0;
    stat_upward_steps_taken_ = 0;
    stat_num_failures_ = 0;
  }

  /** @} */

  //! Set the score threshold.
  //* An optimization is terminated if the score drops below this value. */
  void set_score_threshold(double s) { min_score_ = s; }

  //! Get the score threshold.
  double get_score_threshold() const { return min_score_; }

  /** Computations can be accelerated by throwing out
      the tails of the distribution of accepted moves. To
      do this, specify a maximum acceptable difference
      between the before and after scores.
  */
  void set_maximum_difference(double d) { max_difference_ = d; }

  double get_maximum_difference() const { return max_difference_; }
  /** @name Movers

       The following methods are used to manipulate the list of Movers.
       Each mover is called at each optimization step, giving it a chance
       to change the current configuration.
       @{
  */
  IMP_LIST_ACTION(public, Mover, Movers, mover, movers, MonteCarloMover *,
                  MonteCarloMovers, {}, {}, {});
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
  bool get_use_incremental_scoring_function() const { return isf_; }
  IncrementalScoringFunction *get_incremental_scoring_function() const {
    return isf_;
  }
  /** @} */
 protected:
  /** Get all movable particles (those that can be moved by the current
      movers.*/
  ParticleIndexes get_movable_particles() const;
  /** Note that if return best is true, this will save the current
      state of the model. Also, if the move is accepted, the
      optimizer states will be updated.
  */
  bool do_accept_or_reject_move(double score, double last,
                                double proposal_ratio);
  bool do_accept_or_reject_move(double score, double proposal_ratio) {
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
  virtual double do_evaluate(const ParticleIndexes &moved,
                             bool force_full_score) const {
    if (isf_) {
      isf_->set_moved_particles(moved);
    }
    if (get_maximum_difference() < NO_MAX) {
      if (score_moved_ && !force_full_score) {
        return get_scoring_function()->evaluate_moved_if_below(
            false, moved, last_energy_ + max_difference_);
      } else {
        return get_scoring_function()->evaluate_if_below(
            false, last_energy_ + max_difference_);
      }
    } else {
      if (score_moved_ && !force_full_score) {
        return get_scoring_function()->evaluate_moved(false, moved);
      } else {
        return get_scoring_function()->evaluate(false);
      }
    }
  }

 private:
  double temp_;
  double last_energy_;
  double best_energy_;
  double max_difference_;
  unsigned int stat_downward_steps_taken_;
  unsigned int stat_upward_steps_taken_;
  unsigned int stat_num_failures_;
  bool return_best_;
  bool score_moved_;
  double min_score_;
  IMP::PointerMember<Configuration> best_;
  ::boost::uniform_real<> rand_;

  Pointer<IncrementalScoringFunction> isf_;
};

//! This variant of Monte Carlo that relaxes after each move
class IMPCOREEXPORT MonteCarloWithLocalOptimization : public MonteCarlo {
  IMP::PointerMember<Optimizer> opt_;
  unsigned int num_local_;

 public:
  MonteCarloWithLocalOptimization(Optimizer *opt, unsigned int steps);

  unsigned int get_number_of_steps() const { return num_local_; }

  Optimizer *get_local_optimizer() const { return opt_; }

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
class IMPCOREEXPORT MonteCarloWithBasinHopping
    : public MonteCarloWithLocalOptimization {
 public:
  MonteCarloWithBasinHopping(Optimizer *opt, unsigned int ns);

 protected:
  virtual void do_step() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MonteCarloWithBasinHopping);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MONTE_CARLO_H */

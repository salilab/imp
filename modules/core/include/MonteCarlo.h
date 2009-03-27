/**
 *  \file MonteCarlo.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_H
#define IMPCORE_MONTE_CARLO_H

#include "config.h"
#include "internal/version_info.h"
#include "Mover.h"

#include <IMP/Optimizer.h>
#include <IMP/container_macros.h>

IMPCORE_BEGIN_NAMESPACE

typedef std::vector<Mover*> Movers;

//! A Monte Carlo optimizer.
/** The optimizer uses a set of Mover objects to propose steps. Currently
    each Mover is called at each Monte Carlo iteration. This may change in
    the future. The movers propose some modification, which is then
    accepted or rejected based on the Metropolis criteria. Optionally, a
    number of local optimization steps are taken before the MonteCarlo step
    is accepted or rejected.

    \note The local optimizer object will never be freed due to lack
    of reference counting with SWIG.

    \see Mover
 */
class IMPCOREEXPORT MonteCarlo: public Optimizer
{
public:
  /** */
  MonteCarlo();
  ~MonteCarlo();

  IMP_OPTIMIZER(internal::version_info)
public:
  //! Return the local optimizer used or NULL
  Optimizer *get_local_optimizer() const {
    return cg_.get();
  }
  //! Set the local optimizer
  /** The number of local steps must be nonzero for the
      local optimizer to be used. Steps taken by the local optimizer
      do not count towards the total number of steps passed to the
      Optimizer::optimize() call.
   */
  void set_local_optimizer(Optimizer* cg);

  //! Set the temperature for the Metropolis criteria
  void set_temperature(Float t) {
    IMP_assert(t>0, "Temperature must be positive");
    temp_=t;
  }
  Float get_temperature() const {
    return temp_;
  }
  //! Stop if the optimization falls below this energy
  void set_energy_threshold(Float t) {
    stop_energy_=t;
  }
  Float get_energy_threshold() const {
    return stop_energy_;
  }
  //! Take this many steps of the local optimizer for each MC step
  /** Steps taken by the local optimizer
      do not count towards the total number of steps passed to the
      Optimizer::optimize() call.
   */
  int get_local_steps() const {
    return num_local_steps_;
  }
  //! Set the probability of each move being made
  /** Make this low if the space is rough and there are many particles.
      The movers should make each individual move with this probability.
      That is, a NormalMover with 100 particles will move each particle
      with probability p.
   */
  void set_move_probability(Float p) {
    IMP_check(p > 0 && p <= 1, "Not a valid probability",
              ValueException);
    probability_=p;
  }

  //! Take this many steps of the local optimizer for each MC step
  void set_local_steps(unsigned int n) {
    num_local_steps_=n;
  }
  //! Return how many times the optimizer has succeeded in taking a step
  unsigned int get_number_of_forward_steps() const {
    return stat_forward_steps_taken_;
  }
  //! Return how many times the optimizer has stepped to higher energy
  unsigned int get_number_of_upward_steps() const {
    return stat_upward_steps_taken_;
  }

  void show(std::ostream &out= std::cout) const;

  /** @name Movers

       The following methods are used to manipulate the list of Movers.
       Each mover is called at each optimization step, giving it a chance
       to change the current configuration.
  */
  /**@{*/
  IMP_LIST(public, Mover, mover, Mover*);
  /**@}*/
private:
  Float temp_;
  Float prior_energy_;
  Float stop_energy_;
  Float probability_;
  WeakPointer<Optimizer> cg_;
  unsigned int num_local_steps_;
  unsigned int stat_forward_steps_taken_;
  unsigned int stat_upward_steps_taken_;
  unsigned int stat_num_failures_;
};

IMP_OUTPUT_OPERATOR(MonteCarlo);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MONTE_CARLO_H */

/**
 *  \file MonteCarlo.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MONTE_CARLO_H
#define __IMP_MONTE_CARLO_H

#include "../Optimizer.h"
#include "../internal/kernel_version_info.h"
#include "Mover.h"

namespace IMP
{

typedef std::vector<Mover*> Movers;

//! A Monte Carlo optimizer.
/** The optimizer uses a set of Mover objects to propose steps. Currently
    each Mover is called at each Monte Carlo iteration. This may change in
    the future. The movers propose some modification, which is then
    accepted or rejected based on the Metropolis criteria. Optionally, a
    number of local optimization steps are taken before the step is
    accepted or rejected.

    \ingroup optimizer
 */
class IMPDLLEXPORT MonteCarlo: public Optimizer
{
public:
  MonteCarlo();
  ~MonteCarlo();

  IMP_OPTIMIZER(internal::kernel_version_info)

  IMP_CONTAINER(Mover, mover, MoverIndex);
public:
  //! Return the local optimizer used or NULL
  Optimizer *get_local_optimizer() const {
    return cg_.get();
  }
  //! Set the local optimizer
  /** The number of local steps must be nonzero for the 
      local optimizer to be used.
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
              ValueException("Not a probability"));
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
private:
  Float temp_;
  Float prior_energy_;
  Float stop_energy_;
  Float probability_;
  internal::ObjectPointer<Optimizer, true> cg_;
  unsigned int num_local_steps_;
  unsigned int stat_forward_steps_taken_;
  unsigned int stat_upward_steps_taken_;
  unsigned int stat_num_failures_;
};

IMP_OUTPUT_OPERATOR(MonteCarlo);

} // namespace IMP

#endif  /* __IMP_MONTE_CARLO_H */

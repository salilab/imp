/**
 *  \file IMP/core/MonteCarloMover.h
 *  \brief The base class for movers for MC optimization.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_MOVER_H
#define IMPCORE_MONTE_CARLO_MOVER_H

#include <IMP/kernel/kernel_config.h>

#include <IMP/kernel/ModelObject.h>
#include <IMP/kernel/Model.h>
#include <IMP/kernel/particle_index.h>
#include <IMP/base/tuple_macros.h>

IMPCORE_BEGIN_NAMESPACE

/** The MonteCarloMoverResult is used as the return value for the
    MonteCarloMover::propose() function. The values are the list of
    particle (indexes) moved and the ratio between the probability of
    the backwards move and the probability of the forwards move (for
    many or most move sets this is 1.0).
*/
IMP_NAMED_TUPLE_2(MonteCarloMoverResult, MonteCarloMoverResults,
                  kernel::ParticleIndexes, moved_particles,
                  double, proposal_ratio,);

//! A base class for classes which perturb particles.
/** Mover objects propose a move, which can then be either accepted or rejected
    based on some criteria. For example, in a Monte-Carlo evaluation scheme.

    The output particles (ModelObject::do_get_outputs()) are assummed to be
    equal to the inputs (ModelObject::do_get_inputs()).
 */
class IMPKERNELEXPORT MonteCarloMover: public kernel::ModelObject
{
  unsigned int num_proposed_;
  unsigned int num_accepted_;
public:
  MonteCarloMover(kernel::Model *m, std::string name);

  //! propose a modification
  /** The method should return the list of all particles that were
      actually moved and the ratio between the forward move probability
      and the backward move probability (for Metropolis-Hastings moves).
      Just return 1.0 for this value if you are not sure.
   */
  MonteCarloMoverResult propose() {
    IMP_OBJECT_LOG;
    set_was_used(true);
    ++num_proposed_;
    return do_propose();
  }

  //! Roll back any changes made to the Particles
  void reject() {
    IMP_OBJECT_LOG;
    do_reject();
  }

  //! Roll back any changes made to the Particles
  void accept() {
    IMP_OBJECT_LOG;
    ++num_accepted_;
    do_accept();
  }

  /** \name Statistics
      Movers keep track of some statistics as they are used.
      @{
  */
  unsigned int get_number_of_proposed() const {
    return num_proposed_;
  }
  unsigned int get_number_of_accepted() const {
    return num_accepted_;
  }
  void reset_statistics() {
    num_proposed_ = 0;
    num_accepted_ = 0;
  }
  /** @} */
protected:
  //! Implement propose_move()
  virtual MonteCarloMoverResult do_propose() = 0;
  //! Implement reset_proposed_move()
  virtual void do_reject() = 0;
  //! Implement accept_proposed_move(), default impl is empty
  virtual void do_accept() {}

  virtual kernel::ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return get_inputs();
  }

  virtual void do_update_dependencies() IMP_OVERRIDE {}
};

IMP_OBJECTS(MonteCarloMover,MonteCarloMovers);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MONTE_CARLO_MOVER_H */

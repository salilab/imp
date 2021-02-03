/**
 *  \file IMP/core/MonteCarloMover.h
 *  \brief The base class for movers for Monte Carlo optimization.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_MOVER_H
#define IMPCORE_MONTE_CARLO_MOVER_H

#include <IMP/core/core_config.h>

#include <IMP/ModelObject.h>
#include <IMP/Model.h>
#include <IMP/particle_index.h>
#include <IMP/tuple_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! Return value of the MonteCarloMover::propose() function.
/** The values are the list of
    particle indexes moved and the ratio between the probability of
    the backwards move and the probability of the forwards move (for
    many or most move sets this is 1.0).
*/
IMP_NAMED_TUPLE_2(MonteCarloMoverResult, MonteCarloMoverResults,
                  ParticleIndexes, moved_particles, double,
                  proposal_ratio, );

//! A base class for classes which perturb particles.
/** Mover objects propose a move, which can then be either accepted or rejected
    based on some criteria. Most commonly this is a Monte Carlo
    evaluation scheme.

    All changed attributes should be optimizable; it is undefined behavior to
    try to optimize an attribute which is not.

    The output particles (ModelObject::do_get_outputs()) are assumed
    to be equal to the inputs (ModelObject::do_get_inputs()).
 */
class IMPCOREEXPORT MonteCarloMover : public ModelObject {
  unsigned int num_proposed_;
  unsigned int num_rejected_;
  bool has_move_;

 public:
  MonteCarloMover(Model *m, std::string name);

  //! Propose a modification
  /** The method should return the list of all particles that were
      actually moved and the ratio between the backward move probability
      and the forward move probability (for Metropolis-Hastings moves).
      Just return 1.0 for this value if you are not sure.
   */
  MonteCarloMoverResult propose() {
    IMP_OBJECT_LOG;
    IMP_USAGE_CHECK(
        !has_move_,
        "Mover already had proposed a move. "
            << " This probably means you added it twice: " << get_name());
    has_move_ = true;
    set_was_used(true);
    ++num_proposed_;
    return do_propose();
  }

  //! Roll back any changes made to the Particles
  void reject() {
    IMP_OBJECT_LOG;
    ++num_rejected_;
    has_move_ = false;
    do_reject();
  }

  //! Accept/commit any changes made to the Particles
  void accept() {
    IMP_OBJECT_LOG;
    has_move_ = false;
    do_accept();
  }

  /** \name Statistics
      Movers keep track of some statistics as they are used.
      @{
  */
  unsigned int get_number_of_proposed() const { return num_proposed_; }
  unsigned int get_number_of_accepted() const {
    return num_proposed_ - num_rejected_;
  }
  void reset_statistics() {
    num_proposed_ = 0;
    num_rejected_ = 0;
  }
  /** @} */
 protected:
  //! Implement propose_move()
  virtual MonteCarloMoverResult do_propose() = 0;
  //! Implement reset_proposed_move()
  virtual void do_reject() = 0;
  //! Implement accept_proposed_move(); default implementation is empty
  virtual void do_accept() {}

  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return get_inputs();
  }
};

IMP_OBJECTS(MonteCarloMover, MonteCarloMovers);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MONTE_CARLO_MOVER_H */

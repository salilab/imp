/**
 *  \file IMP/core/Mover.h
 *  \brief The base class for movers for MC optimization.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MOVER_H
#define IMPCORE_MOVER_H

#include <IMP/core/core_config.h>

#include <IMP/base_types.h>
#include <IMP/RefCounted.h>
#include <IMP/WeakPointer.h>
#include <IMP/ModelObject.h>
#include <IMP/Optimizer.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

class MonteCarlo;

//! A base class for classes which perturb particles.
/** Mover objects are designed primarily to be used with
    the Monte Carlo optimizer. You probably want to use MoverBase
    if you are implementing a Mover.
    \see MonteCarlo
 */
class IMPCOREEXPORT Mover: public IMP::ModelObject
{
  friend class MonteCarlo;
  UncheckedWeakPointer<Optimizer> opt_;
public:
  Mover(Model *m, std::string name);

  //! propose a modification
  /** \param[in] size A number between 0 and 1 used to scale the proposed
      moves. This number can be either used to scale a continuous move
      or affect the probability of a discrete move.

      The method should return the list of all particles that were
      actually moved.
   */
  virtual ParticlesTemp propose_move(Float size)=0;

  //! Roll back any changes made to the Particles
  virtual void reset_move()=0;

  //! Return the set of particles over which moves can be proposed
  virtual ParticlesTemp get_output_particles() const=0;

  //! Get a pointer to the optimizer which has this mover.
  Optimizer *get_optimizer() const {
    IMP_CHECK_OBJECT(this);
    return opt_;
  }
  void set_optimizer(Optimizer *c) {
    if (c) set_was_used(true);
    opt_=c;
  }
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  virtual void do_update_dependencies() IMP_OVERRIDE {
  }
  IMP_REF_COUNTED_DESTRUCTOR(Mover);
};

IMP_OBJECTS(Mover,Movers);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MOVER_H */

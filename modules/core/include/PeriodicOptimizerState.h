/**
 *  \file IMP/core/PeriodicOptimizerState.h
 *  \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_PERIODIC_OPTIMIZER_STATE_H
#define IMPCORE_PERIODIC_OPTIMIZER_STATE_H

#include <IMP/core/core_config.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>
IMPCORE_BEGIN_NAMESPACE

/** This optimizer state calls its do_update() method with a given
    period.*/
class IMPCOREEXPORT PeriodicOptimizerState : public OptimizerState
{
  unsigned int period_, call_number_, update_number_;
 public:
  PeriodicOptimizerState(std::string name);
  /** Called when an optimization begins. It resets the current call number
      as well as making sure that the last frame is written.*/
  IMP_IMPLEMENT(void set_is_optimizing(bool tf));
  void set_period(unsigned int p);
  unsigned int get_period() const;
  /** Reset everything to look like the first call.*/
  virtual void reset();
  IMP_IMPLEMENT(virtual void update());
  /** Force the optimizer state to perform its action now.
   */
  void update_always();
  //! Return the number of times update has been called
  unsigned int get_number_of_updates() const {
    return update_number_;
  }
  //! Set the counter
  void set_number_of_updates(unsigned int n) {
    update_number_=n;
  }
protected:
  /** This method is called every get_period() update calls.*/
  virtual void do_update(unsigned int call_number)=0;
};

IMP_OBJECTS(PeriodicOptimizerState, PeriodicOptimizerStates);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PERIODIC_OPTIMIZER_STATE_H */

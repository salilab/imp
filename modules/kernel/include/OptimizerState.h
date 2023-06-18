/**
 *  \file IMP/OptimizerState.h   \brief Shared optimizer state.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OPTIMIZER_STATE_H
#define IMPKERNEL_OPTIMIZER_STATE_H

#include <IMP/kernel_config.h>
#include "ModelObject.h"
#include <IMP/WeakPointer.h>
#include <IMP/Object.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <iostream>

IMPKERNEL_BEGIN_NAMESPACE

class Optimizer;

//! Shared optimizer state that is invoked upon commitment of new coordinates.
/** An OptimizerState update() method is called every time that an
    owning Optimizer commits to a new set of coordinates. (For example, this
    is typically every step during molecular dynamics, or every accepted move
    during Monte Carlo.) The update() method, in turn, invokes do_update(),
    which can be overridden by inheriting classes.

    @note An OptimizerState may have periodicity by its set_period() method.

    @note An OptimizerState is added to an Optimizer object by calling
          Optimizer::add_optimizer_state().

    @note An OptimizerState may change the values of particle
          attributes. However, changes to whether an attribute is optimized
          or not may not be picked up by the Optimizer until the next call
          to Optimizer::optimize().

    \note When logging is VERBOSE, the state should print enough information
          in update() to reproduce the entire flow of data in update. When
          logging is TERSE the state should print out only a constant number
          of lines per update call.
 */
class IMPKERNELEXPORT OptimizerState : public ModelObject {
  friend class Optimizer;
  unsigned int period_, call_number_, update_number_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<ModelObject>(this), period_, call_number_,
       update_number_, is_optimizing_);
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      optimizer_ = nullptr;
    }
  }

  void set_optimizer(Optimizer* optimizer);

 public:
  //! Constructor.
  /** Constructs an optimizer state whose update() method is invoked
      every time that a set of model coordinates is committed
      by an optimizer.

      @param m the model to which this optimizer state is associated
      @param name the name of the object
      @note An OptimizerState may become periodic via its set_period()
            method.
  */
  OptimizerState(Model* m, std::string name);
  OptimizerState();

  //! Called when the Optimizer accepts a new conformation
  /**
      This method is called by owning optimizers every time they commit.
      However, if set_period(p) was invoked, it calls do_update() only
      every p times it is called (by any optimizer).

      @note Overriding this method is deprecated; override do_update() instead.
  */
  virtual void update();

  //! Called by an Optimizer to signal begin/end of an optimize run.
  /** At the beginning of an optimize run, set_is_optimizing(true) is
      called. At the end, set_is_optimizing(false) is called.

      \note Do not override; override do_set_is_optimizing() instead.
  */
  virtual void set_is_optimizing(bool);

  Optimizer* get_optimizer() const {
    IMP_INTERNAL_CHECK(optimizer_,
                       "Must call set_optimizer before get_optimizer on state");
    return optimizer_.get();
  }

  //! Set the periodicity of this state.
  /** This causes update() to invoke do_update() only every p calls
      to update() rather than on every call (p=1). Note that this
      periodicity is shared by all optimizers that own this
      OptimizerState object.

      @param p periodicity
   */
  void set_period(unsigned int p);

  //! Get the periodicity of this state.
  /** @return the periodicity of this state (how many calls to update()
              are required to invoke do_update())
   */
  unsigned int get_period() const { return period_; }

  //! Reset counters, as if at the start of an optimize run.
  virtual void reset();

  //! Force the state to perform its action now, ignoring the periodicity.
  void update_always();

  //! Return the number of times do_update() has been called
  unsigned int get_number_of_updates() const { return update_number_; }

  //! Set the counter of number of times do_update() has been called
  void set_number_of_updates(unsigned int n) { update_number_ = n; }

  IMP_REF_COUNTED_DESTRUCTOR(OptimizerState);

 protected:
  /** This method is called every get_period() update calls. The number of
      times this method has been called since the last reset or start of the
      optimization run is passed.*/
  virtual void do_update(unsigned int) { update(); }

  virtual void do_set_is_optimizing(bool) {}

  virtual ModelObjectsTemp do_get_inputs() const override {
    return ModelObjectsTemp();
  }
  virtual ModelObjectsTemp do_get_outputs() const override IMP_SWIG_FINAL {
    return ModelObjectsTemp();
  }

 private:
  UncheckedWeakPointer<Optimizer> optimizer_;
  bool is_optimizing_;
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_OPTIMIZER_STATE_H */

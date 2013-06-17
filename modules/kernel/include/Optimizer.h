/**
 *  \file IMP/kernel/Optimizer.h     \brief Base class for all optimizers.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OPTIMIZER_H
#define IMPKERNEL_OPTIMIZER_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "VersionInfo.h"
#include <IMP/base/Object.h>
#include "utility.h"
#include "Model.h"
#include "Particle.h"
#include <IMP/base/Pointer.h>
#include "OptimizerState.h"
#include <IMP/base/Vector.h>
#include <limits>
#include <cmath>

IMPKERNEL_BEGIN_NAMESPACE

//! Base class for all optimizers.
/** An optimizer attempts to improve the current configuration of the
    Model by moving particles around so as to lower the score.

    The Optimizer maintains a list of OptimizerStates which are
    updated each time the conformation is changed.

    The optimizers have one key method Optimizer::optimize() which takes
    the number of steps to perform. The optimizers can have other
    stopping conditions as appropriate.

    A typical optimization loop proceeds by:
    - the optimizer calls Model::evaluate() to compute the score
    (and possibly the derivatives) of the
    current conformation of the Model.
    - the optimizer uses this information to update the optimizeable
    parameters of the Particles contained in the Model.
*/
class IMPKERNELEXPORT Optimizer : public IMP::base::Object {
 public:
  /** \deprecated_at{2.1} Use the constructor with a Model and a name.*/
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  Optimizer();
  Optimizer(Model *m, std::string name = "Optimizer %1%");

  //! Optimize the model for up to max_steps iterations
  /** Optimize the model

      @param[in] max_steps The maximum number of iterations of the
      optimizer to perform. Increasing this number will generally make
      the optimizer spend more time searching for a solution, but
      beyond that, the details of what changes will vary.

      @return The final score.
  */
  double optimize(unsigned int max_steps);

  /** \deprecated_at{2.1} Do not use as it is not reliably supported. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  void set_score_threshold(double s) {
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Not a reliable function.");
    min_score_ = s;
  }
  /** \deprecated_at{2.1} Do not use as it is not reliably supported. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  double get_score_threshold() const {
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Not a reliable function.");
    return min_score_;
  }

  /** Optimization can be stopped if all the thresholds in the Model are
      satisfied. */
  void set_stop_on_good_score(bool tf) { stop_on_good_score_ = tf; }
  bool get_stop_on_good_score() const { return stop_on_good_score_; }
  //! Return the score found in the last evaluate
  double get_last_score() const { return cache_->get_last_score(); }

  //! Return the scoring function that is being used
  ScoringFunction *get_scoring_function() const { return cache_; }

  //! Get the model being optimized
  Model *get_model() const { return model_.get(); }

  //! \deprecated_at{2.1} Use the constructor that takes the model
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  void set_model(Model *m);

    /** @name States

        The stored OptimizerState objects are updated each time the
        Optimizer decides to accept a new configuration of the Model.
        To manipulate the list of optimizer states use the methods below.
    */
    /**@{*/
  IMP_LIST_ACTION(public, OptimizerState, OptimizerStates, optimizer_state,
                  optimizer_states, OptimizerState *, OptimizerStates, {
    set_optimizer_state_optimizer(obj, this);
    obj->set_was_used(true);
  },
                  {}, {
    Optimizer::set_optimizer_state_optimizer(obj, nullptr);
  });
  /**@}*/

  /** By default, the Optimizer uses the scoring function provided by
      the model, but you can use another scoring function instead.
  */
  virtual void set_scoring_function(ScoringFunctionAdaptor sf);

  /** \deprecated_at{2.1} Use set_scoring_function() instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  void set_restraints(const RestraintsTemp &rs);

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Optimizer);

 protected:
  //! override this function to do actual optimization
  virtual double do_optimize(unsigned int ns) = 0;
  //! Update optimizer states, should be called at each successful step
  /** Make sure the scoring function restraints are up to date before this is
      called (eg by calling evaluate).*/
  void update_states() const;

#ifndef SWIG
  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  FloatIndexes get_optimized_attributes() const;
  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  void set_value(FloatIndex fi, double v) const;

  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  Float get_value(FloatIndex fi) const;

  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  Float get_derivative(FloatIndex fi) const;

//!@}

#if !defined(SWIG)
  /** \deprecated_at{2.1} Use get_width instead.*/
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  double width(FloatKey k) const;
#endif
  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  double get_width(FloatKey k) const;
  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  void set_scaled_value(FloatIndex fi, Float v) const;

  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  double get_scaled_value(FloatIndex fi) const;

  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  double get_scaled_derivative(FloatIndex fi) const;

  /** \deprecated_at{2.1} Use AttributeOptimizer instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  void clear_range_cache();
#endif  // SWIG

  /** \deprecated_at{2.1} Use Optimizer::get_scoring_function() instead. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  Restraints get_restraints() const;

 private:
  void set_is_optimizing_states(bool tf) const;
  static void set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o);
  mutable Floats widths_;
  base::Pointer<Model> model_;
  double min_score_;
  bool stop_on_good_score_;
  base::Pointer<ScoringFunction> cache_;
};

IMP_OBJECTS(Optimizer, Optimizers);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_OPTIMIZER_H */

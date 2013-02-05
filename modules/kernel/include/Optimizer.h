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
#include "Object.h"
#include "utility.h"
#include "Model.h"
#include "Particle.h"
#include "Pointer.h"
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

    \implementationwithoutexample{Optimizer, IMP_OPTIMIZER}
*/
class IMPKERNELEXPORT Optimizer: public IMP::base::Object
{
 public:
  Optimizer();
  Optimizer(Model *m, std::string name="Optimizer %1%");

  //! Optimize the model for up to max_steps iterations
  /** Optimize the model

      @param[in] max_steps The maximum number of iterations of the
      optimizer to perform. Increasing this number will generally make
      the optimizer spend more time searching for a solution, but
      beyond that, the details of what changes will vary.

      @return The final score.
  */
  double optimize(unsigned int max_steps);

#ifndef IMP_DOXYGEN
  /** \name Score threshold
      Optimizers can be set to stop if they achieve a score below
      a score threshold. This is useful so that they don't spend time
      improving already very good solutions.
      @{
  */
  void set_score_threshold(double s) {min_score_=s;}
  double get_score_threshold() const {return min_score_;}
  /** @} */
#endif

  /** Optimization can be stopped if all the thresholds in the Model are
      satisfied. */
  void set_stop_on_good_score(bool tf) {
    stop_on_good_score_=tf;
  }
  bool get_stop_on_good_score() const {
    return stop_on_good_score_;
  }
  //! Return the score found in the last evaluate
  double get_last_score() const {
    return cache_->get_last_score();
  }

  //! Return the scoring function that is being used
  ScoringFunction *get_scoring_function() const {
    return cache_;
  }

  //! Get the model being optimized
  Model *get_model() const {
    return model_.get();
  }

  //! Set the model being optimized
  /**
     \note The model is not owned by the optimizer and so is not
     deleted when the optimizer is deleted. Further, the Optimizer
     does not prevent the model from being deleted when all Python
     references go away.
  */
  void set_model(Model *m);

  //! Print info about the optimizer state
  /** It should end in a newline */
  virtual void show(std::ostream &out= std::cout) const {
    out << "Some optimizer" << std::endl;
  }

  /** @name States

      The stored OptimizerState objects are updated each time the
      Optimizer decides to accept a new configuration of the Model.
      To manipulate the list of optimizer states use the methods below.
  */
  /**@{*/
  IMP_LIST_ACTION(public, OptimizerState, OptimizerStates,
                  optimizer_state, optimizer_states, OptimizerState*,
                  OptimizerStates,
                  {
                    set_optimizer_state_optimizer(obj, this);
                    obj->set_was_used(true);
                  },{},
                  {Optimizer::set_optimizer_state_optimizer(obj, nullptr);});
  /**@}*/

  /** By default, the Optimizer uses the scoring function provided by
      the model, but you can use another scoring function instead.
  */
  virtual void set_scoring_function(ScoringFunctionAdaptor sf);

#ifndef IMP_DOXYGEN
  void set_restraints(const RestraintsTemp &rs);
#endif

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Optimizer);

protected:
  //! override this function to do actual optimization
  virtual double do_optimize(unsigned int ns) =0;
  //! Update optimizer states, should be called at each successful step
  /** Make sure the scoring function restraints are up to date before this is
      called (eg by calling evaluate).*/
  void update_states() const;

  /** @name Methods for getting and setting optimized attributes
      Optimizers don't have to go through the particles themselves
      looking for values to optimize unless they care about special
      properties of the optimized values. Instead they can iterate
      through the list of optimized attributes, each of which is
      identified by a FloatIndex. With these FloatIndex objects
      they can get and set the values and derivatives as needed.
  */
  //!@{
  FloatIndexes get_optimized_attributes() const {
    return get_model()->get_optimized_attributes();
  }
  IMP_PROTECTED_METHOD(void, set_value,(FloatIndex fi, double v), const, {
      get_model()->set_attribute(fi.get_key(), fi.get_particle(), v);
    });

  IMP_PROTECTED_METHOD(Float, get_value,(FloatIndex fi), const, {
    return get_model()->get_attribute(fi.get_key(), fi.get_particle());
    });

  IMP_PROTECTED_METHOD(Float, get_derivative,(FloatIndex fi), const, {
    return get_model()->get_derivative(fi.get_key(), fi.get_particle());
    });

  //!@}

  IMP_PROTECTED_METHOD(double, width,(FloatKey k), const, {
    if (widths_.size() <=k.get_index() || widths_[k.get_index()]==0) {
      FloatRange w= model_->get_range(k);
      double wid=static_cast<double>(w.second)- w.first;
      widths_.resize(std::max(widths_.size(), size_t(k.get_index()+1)), 0.0);
      if (wid > .0001) {
        //double nwid= std::pow(2, std::ceil(log2(wid)));
        widths_[k.get_index()]= wid;
      } else {
        widths_[k.get_index()]= 1.0;
      }
    }
    return widths_[k.get_index()];
    //return 1.0;
    });

  /** @name Methods to get and set scaled optimizable values
      Certain optimizers benefit from having all the optimized values
      scaled to vary over a similar range. These accessors use the
      Model::get_range ranges to scale the values before returning
      them and unscale them before setting them.
  */
  //{@
  IMP_PROTECTED_METHOD(void, set_scaled_value,(FloatIndex fi, Float v),
                       const, {
                         double wid = width(fi.get_key());
                         set_value(fi, v*wid);
                       });

  IMP_PROTECTED_METHOD(double, get_scaled_value,(FloatIndex fi),
                       const,  {
                         double uv= get_value(fi);
                         double wid = width(fi.get_key());
                         return uv/wid;
                       });

  IMP_PROTECTED_METHOD(double, get_scaled_derivative,(FloatIndex fi),
                       const, {
                         double uv=get_derivative(fi);
                         double wid= width(fi.get_key());
                         return uv*wid;
                       });

  //! Clear the cache of range information. Do this at the start of optimization
  IMP_PROTECTED_METHOD(void, clear_range_cache,(),, {
      widths_.clear();
    });
  //!@}

#ifndef IMP_DOXYGEN
  //! Return the restraint sets used in evaluation.
  /** Use IMP::kernel::get_restraints() to get the actual restraints used.
   */
    IMP_PROTECTED_METHOD(Restraints, get_restraints, (), const,);
#endif
 private:
  void set_is_optimizing_states(bool tf) const;
  static void set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o);
  mutable Floats widths_;
  Pointer<Model> model_;
  double min_score_;
  bool stop_on_good_score_;
  Pointer<ScoringFunction> cache_;
};


IMP_OBJECTS(Optimizer,Optimizers);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_OPTIMIZER_H */

/**
 *  \file Optimizer.h     \brief Base class for all optimizers.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_OPTIMIZER_H
#define IMP_OPTIMIZER_H

#include "kernel_config.h"
#include "base_types.h"
#include "VersionInfo.h"
#include "Object.h"
#include "utility.h"
#include "Model.h"
#include "Particle.h"
#include "Pointer.h"
#include "OptimizerState.h"
#include <limits>
#include <cmath>

IMP_BEGIN_NAMESPACE

//! Base class for all optimizers.
/** The Optimizer maintains a list of OptimizerStates which are
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
class IMPEXPORT Optimizer: public Object
{
 public:
  Optimizer(Model *m= NULL, std::string name="Optimizer %1%");

  /** Optimize the model

      \param[in] max_steps The maximum number of iterations of the
      optimizer to perform. Increasing this number will generally make
      the optimizer spend more time searching for a solution, but
      beyond that, the details of what changes will vary.

      \return The final score.
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
    return last_score_;
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
  void set_model(Model *m) {model_=m;}

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
                    Optimizer::set_optimizer_state_optimizer(obj, this);
                    obj->set_was_used(true);
                  },{},
                  {Optimizer::set_optimizer_state_optimizer(obj, NULL);});
  /**@}*/

  /** \brief The optimizer can be told to use only a subset of the
      restraints to compute scores.

      These restraints must all be part of the model. Passing
      an empty list will revert to using the whole model.
  */
  void set_restraints(const RestraintsTemp &rs);


  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Optimizer);

 protected:
  //! Evaluate the score of the model (or of a subset of the restraints
  //! if desired.
  double evaluate(bool compute_derivatives) const;

  //! Evaluate the score of the model (or of a subset of the restraints
  //! if desired.
  double evaluate_if_below(bool compute_derivatives,
                          double max) const;


  //! override this function to do actual optimization
  virtual double do_optimize(unsigned int ns) =0;
  //! Update optimizer state, should be called at each successful step
  void update_states() const ;



  /** @name Methods for getting and setting optimized attributes
      Optimizers don't have to go through the particles themselves
      looking for values to optimize unless they care about special
      properties of the optimized values. Instead they can iterate
      through the list of optimized attributes, each of which is
      identified by a FloatIndex. With these FloatIndex objects
      they can get and set the values and derivatives as needed.
  */
  //!@{
  typedef Model::FloatIndex FloatIndex;
  typedef std::vector<FloatIndex> FloatIndexes;
  std::vector<Model::FloatIndex> get_optimized_attributes() const {
    return get_model()->get_optimized_attributes();
  }
  void set_value(Model::FloatIndex fi, double v) const {
    get_model()->set_attribute(fi.k_, fi.p_, v);
  }

  Float get_value(Model::FloatIndex fi) const {
    return get_model()->get_attribute(fi.k_, fi.p_);
  }

  Float get_derivative(Model::FloatIndex fi) const {
    return get_model()->get_derivative(fi.k_, fi.p_);
  }

  //!@}

  double width(FloatKey k) const {
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
  }

  /** @name Methods to get and set scaled optimizable values
      Certain optimizers benefit from having all the optimized values
      scaled to vary over a similar range. These accessors use the
      Model::get_range ranges to scale the values before returning
      them and unscale them before setting them.
  */
  //{@
  void set_scaled_value(Model::FloatIndex fi, Float v) const {
    double wid = width(fi.k_);
    set_value(fi, v*wid);
  }

  double get_scaled_value(Model::FloatIndex fi) const  {
    double uv= get_value(fi);
    double wid = width(fi.k_);
    return uv/wid;
  }

  double get_scaled_derivative(Model::FloatIndex fi) const {
    double uv=get_derivative(fi);
    double wid= width(fi.k_);
    return uv*wid;
  }

  //! Clear the cache of range information. Do this at the start of optimization
  void clear_range_cache() {
    widths_.clear();
  }
  //!@}

  //! Return the restraint sets used in evaluation.
  /** Use IMP::get_restraints() to get the actual restraints used.
   */
  RestraintsTemp get_restraints() const;

 private:
  static void set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o);

  mutable Floats widths_;
  Pointer<Model> model_;
  double min_score_;
  bool stop_on_good_score_;
  Restraints restraints_;
  RestraintsTemp flattened_restraints_;
  mutable double last_score_;
};


IMP_OBJECTS(Optimizer,Optimizers);

IMP_END_NAMESPACE

#endif  /* IMP_OPTIMIZER_H */

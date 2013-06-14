/**
 *  \file IMP/kernel/AttributeOptimizer.h
 *   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_ATTRIBUTE_OPTIMIZER_H
#define IMPKERNEL_ATTRIBUTE_OPTIMIZER_H

#include <IMP/kernel/kernel_config.h>
#include "Optimizer.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Base class for optimizers that act on individual attributes.
/** AttributeOptimizers optimize the collection of optimized
    attributes (see Model::set_is_optimized()) in contrast to,
    say molecular dynamics where the fundamental entity is a Particle.
*/
class IMPKERNELEXPORT AttributeOptimizer : public Optimizer {
 public:
 /** \deprecated_at{2.1} Use the constructor with a Model and a name.*/
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  AttributeOptimizer();
  AttributeOptimizer(Model *m, std::string name = "Optimizer %1%");

protected:

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
  void set_value(FloatIndex fi, double v) const {
    get_model()->set_attribute(fi.get_key(), fi.get_particle(), v);
  }

 Float get_value(FloatIndex fi) const {
    return get_model()->get_attribute(fi.get_key(), fi.get_particle());
  }

  Float get_derivative(FloatIndex fi) const {
    return get_model()->get_derivative(fi.get_key(), fi.get_particle());
  }

    //!@}

  double get_width(FloatKey k) const {
    if (widths_.size() <= k.get_index() || widths_[k.get_index()] == 0) {
      FloatRange w = get_model()->get_range(k);
      double wid = static_cast<double>(w.second) - w.first;
      widths_.resize(std::max(widths_.size(), size_t(k.get_index() + 1)), 0.0);
      if (wid > .0001) {
        //double nwid= std::pow(2, std::ceil(log2(wid)));
        widths_[k.get_index()] = wid;
      } else {
        widths_[k.get_index()] = 1.0;
      }
    }
    return widths_[k.get_index()];
  }

    /** @name Methods to get and set scaled optimizable values
        Certain optimizers benefit from having all the optimized values
        scaled to vary over a similar range. These accessors use the
        Model::get_range ranges to scale the values before returning
        them and unscale them before setting them.
    */
    //{@
  void set_scaled_value (FloatIndex fi, Float v) const {
    double wid = get_width(fi.get_key());
    set_value(fi, v * wid);
  }

  double get_scaled_value (FloatIndex fi) const {
    double uv = get_value(fi);
    double wid = get_width(fi.get_key());
    return uv / wid;
  }

  double get_scaled_derivative(FloatIndex fi) const {
    double uv = get_derivative(fi);
    double wid = get_width(fi.get_key());
    return uv * wid;
  }

    //! Clear the cache of range information. Do this at the start of
    //optimization
  void clear_range_cache() {
    widths_.clear();
  }
//!@}
 private:
  mutable Floats widths_;
};

IMP_OBJECTS(AttributeOptimizer, AttributeOptimizers);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_ATTRIBUTE_OPTIMIZER_H */

/**
 *  \file IMP/AttributeOptimizer.h
 *  \brief Base class for optimizers that act on individual attributes.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_ATTRIBUTE_OPTIMIZER_H
#define IMPKERNEL_ATTRIBUTE_OPTIMIZER_H

#include <IMP/kernel_config.h>
#include "Optimizer.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPKERNEL_BEGIN_NAMESPACE

//! Base class for optimizers that act on individual attributes.
/** AttributeOptimizers optimize the collection of optimized
    attributes (see Model::set_is_optimized()) in contrast to,
    say molecular dynamics where the fundamental entity is a Particle.
*/
class IMPKERNELEXPORT AttributeOptimizer : public Optimizer {
 public:
  AttributeOptimizer(Model *m, std::string name = "Optimizer %1%");
  AttributeOptimizer() {}

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
    Model *m = get_model();
    return m->get_optimized_attributes()
      + m->internal::Vector3DDerivAttributeTable::get_optimized_vector_attributes()
      + m->internal::Vector4DDerivAttributeTable::get_optimized_vector_attributes();
  }
  void set_value(FloatIndex fi, double v) const {
    fi.set_value(get_model(), v);
  }

  Float get_value(FloatIndex fi) const {
    return fi.get_value(get_model());
  }

  Float get_derivative(FloatIndex fi) const {
    return fi.get_derivative(get_model());
  }

  //!@}

  double get_width(FloatKey k) const {
    if (widths_.size() <= k.get_index() || widths_[k.get_index()] == 0) {
      FloatRange w = get_model()->get_range(k);
      double wid = static_cast<double>(w.second) - w.first;
      widths_.resize(std::max(widths_.size(), size_t(k.get_index() + 1)), 0.0);
      if (wid > .0001) {
        // double nwid= std::pow(2, std::ceil(log2(wid)));
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

      For now, Vector3D and Vector4D values are not scaled.
  */
  //{@
  void set_scaled_value(FloatIndex fi, Float v) const {
    FloatKey k = fi.get_float_key();
    if (k == FloatKey()) {
      set_value(fi, v);
    } else {
      double wid = get_width(k);
      set_value(fi, v * wid);
    }
  }

  double get_scaled_value(FloatIndex fi) const {
    double uv = get_value(fi);
    FloatKey k = fi.get_float_key();
    if (k == FloatKey()) {
      return uv;
    } else {
      double wid = get_width(fi.get_float_key());
      return uv / wid;
    }
  }

  double get_scaled_derivative(FloatIndex fi) const {
    double uv = get_derivative(fi);
    FloatKey k = fi.get_float_key();
    if (k == FloatKey()) {
      return uv;
    } else {
      double wid = get_width(fi.get_float_key());
      return uv * wid;
    }
  }

  //! Clear the cache of range information. Do this at the start of
  // optimization
  void clear_range_cache() { widths_.clear(); }
  //!@}
 private:
  mutable Floats widths_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Optimizer>(this));
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      clear_range_cache();
    }
  }
};

IMP_OBJECTS(AttributeOptimizer, AttributeOptimizers);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_ATTRIBUTE_OPTIMIZER_H */

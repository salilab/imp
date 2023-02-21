/**
 *  \file IMP/core/Linear.h    \brief A linear function.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_LINEAR_H
#define IMPCORE_LINEAR_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

IMPCORE_BEGIN_NAMESPACE

//! %Linear function
/** \note The offset is not meaningful for optimization, but does
    make the displayed energies nicer.
 */
class Linear : public UnaryFunction {
 public:
  //! Create with the given offset and slope.
  Linear(double offset, double slope) : slope_(slope), offset_(offset) {}
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  Linear() {}
#endif

  void set_slope(double f) { slope_ = f; }

  void set_offset(double f) { offset_ = f; }

  virtual double evaluate(double feature) const override {
    return (feature - offset_) * slope_;
  }

  virtual DerivativePair evaluate_with_derivative(double feature) const
      override {
    return DerivativePair(evaluate(feature), slope_);
  }

  IMP_OBJECT_METHODS(Linear);

 private:
  double slope_, offset_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<UnaryFunction>(this),
       slope_, offset_);
  }
};

IMPCORE_END_NAMESPACE

CEREAL_REGISTER_TYPE_WITH_NAME(IMP::core::Linear, "core.Linear");

#endif /* IMPCORE_LINEAR_H */

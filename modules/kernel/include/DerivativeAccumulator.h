/**
 *  \file IMP/DerivativeAccumulator.h
 *  \brief Class for adding derivatives from restraints to the model.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DERIVATIVE_ACCUMULATOR_H
#define IMPKERNEL_DERIVATIVE_ACCUMULATOR_H

#include <IMP/kernel_config.h>
#include <IMP/showable_macros.h>
#include <IMP/check_macros.h>
#include <IMP/value_macros.h>
#include <IMP/math.h>
#include <IMP/exception.h>
#include <IMP/VectorD.h>

IMPKERNEL_BEGIN_NAMESPACE

//! Class for adding derivatives from restraints to the model.
/** This class was created so that restraints can be weighted using
    a RestraintSet and that the derivatives would be scaled appropriately */
class IMPKERNELEXPORT DerivativeAccumulator {
 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(DerivativeAccumulator);
  //! the weight is one by default
  DerivativeAccumulator(double weight = 1.0) : weight_(weight) {}

  //! The weight is multiplied by the new weight
  DerivativeAccumulator(const DerivativeAccumulator &copy, double weight)
      : weight_(copy.weight_ * weight) {}

  //! Scale a float value appropriately.
  /** \param[in] value Value to add to the float attribute derivative.
   */
  double operator()(const double value) const {
    IMP_INTERNAL_CHECK(!isnan(value), "Can't set derivative to NaN.");
    return value * weight_;
  }

  //! Scale a Vector3D value appropriately.
  /** \param[in] value Value to add to the Vector3D attribute derivative.
   */
  Vector3D operator()(const Vector3D &value) const {
    return Vector3D(operator()(std::get<0>(value)),
                    operator()(std::get<1>(value)),
                    operator()(std::get<2>(value)));
  }

  //! Scale a Vector4D value appropriately.
  /** \param[in] value Value to add to the Vector4D attribute derivative.
   */
  Vector4D operator()(const Vector4D &value) const {
    return Vector4D(operator()(std::get<0>(value)),
                    operator()(std::get<1>(value)),
                    operator()(std::get<2>(value)),
                    operator()(std::get<3>(value)));
  }

  double get_weight() const { return weight_; }
  IMP_SHOWABLE_INLINE(DerivativeAccumulator, out << weight_);

 private:
  double weight_;
};

IMP_VALUES(DerivativeAccumulator, DerivativeAccumulators);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_DERIVATIVE_ACCUMULATOR_H */

/**
 *  \file IMP/kernel/DerivativeAccumulator.h
 *  \brief Class for adding derivatives from
 *                                         restraints to the model.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DERIVATIVE_ACCUMULATOR_H
#define IMPKERNEL_DERIVATIVE_ACCUMULATOR_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/showable_macros.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/value_macros.h>
#include <IMP/base/math.h>
#include <IMP/base/exception.h>

IMPKERNEL_BEGIN_NAMESPACE

//! Class for adding derivatives from restraints to the model.
/** This class was created so that restraints can be weighted using
    a RestraintSet and that the derivatives would be scaled appropriately */
class IMPKERNELEXPORT DerivativeAccumulator
{
public:
  //! the weight is one by default
  DerivativeAccumulator(double weight=1.0)
      : weight_(weight) {}

  //! The weight is multiplied by the new weight
  DerivativeAccumulator(const DerivativeAccumulator &copy, double weight=1.0)
      : weight_(copy.weight_ * weight) {}

  //! Scale a value appropriately.
  /** \param[in] value Value to add to the float attribute derivative.
   */
  double operator()(const double value) const {
    IMP_INTERNAL_CHECK(!base::isnan(value),
                       "Can't set derivative to NaN.");
    return value * weight_;
  }
  double get_weight() const {return weight_;}
  IMP_SHOWABLE_INLINE(DerivativeAccumulator, out << weight_);

private:
  double weight_;
};

IMP_VALUES(DerivativeAccumulator, DerivativeAccumulators);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_DERIVATIVE_ACCUMULATOR_H */

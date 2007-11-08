/**
 *  \file DerivativeAccumulator.h   \brief Class for adding derivatives from
 *                                         restraints to the model.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DERIVATIVE_ACCUMULATOR_H
#define __IMP_DERIVATIVE_ACCUMULATOR_H

#include "IMP_config.h"
#include "Base_Types.h"
#include "ModelData.h"

namespace IMP
{

//! Class for adding derivatives from restraints to the model.
class IMPDLLEXPORT DerivativeAccumulator
{
public:
  DerivativeAccumulator(ModelData *model_data, float weight=1.0)
      : model_data_(model_data), weight_(weight) {}

  DerivativeAccumulator(const DerivativeAccumulator &copy, float weight=1.0)
      : model_data_(copy.model_data_), weight_(copy.weight_ * weight) {}

  //! Add value to derivative.
  /** \param[in] idx Index of the particle float attribute.
      \param[in] value Value to add to the float attribute derivative.
  */
  void add_to_deriv(const FloatIndex idx, const Float value) {
    model_data_->add_to_deriv(idx, value * weight_);
  }

protected:
  ModelData *model_data_;
  float weight_;
};

} // namespace IMP

#endif  /* __IMP_DERIVATIVE_ACCUMULATOR_H */

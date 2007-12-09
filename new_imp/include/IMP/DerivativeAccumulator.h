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
#include "base_types.h"
#include "ModelData.h"

namespace IMP
{

//! Class for adding derivatives from restraints to the model.
class IMPDLLEXPORT DerivativeAccumulator
{
public:
  DerivativeAccumulator(Float weight=1.0)
      : weight_(weight) {}

  DerivativeAccumulator(const DerivativeAccumulator &copy, Float weight=1.0)
      : weight_(copy.weight_ * weight) {}

  //! Scale a value appropriately.
  /** \param[in] value Value to add to the float attribute derivative.
   */
  Float operator()(const Float value) const {
    return value * weight_;
  }

protected:
  Float weight_;
};

} // namespace IMP

#endif  /* __IMP_DERIVATIVE_ACCUMULATOR_H */

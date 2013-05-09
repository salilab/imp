/**
 *  \file IMP/core/Linear.h    \brief A linear function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_LINEAR_H
#define IMPCORE_LINEAR_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/unary_function_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! %Linear function
/** \note The offset is not meaningful for optimization, but does
    make the displayed energies nicer.
 */
class Linear : public UnaryFunction {
 public:
  //! Create with the given offset and slope.
  Linear(double offset, double slope) : slope_(slope), offset_(offset) {}

  IMP_UNARY_FUNCTION_INLINE(
      Linear, (feature - offset_) * slope_, slope_,
      "Linear: " << slope_ << ", " << offset_ << std::endl);
  void set_slope(double f) { slope_ = f; }

  void set_offset(double f) { offset_ = f; }

 private:
  double slope_, offset_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_LINEAR_H */

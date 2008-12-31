/**
 *  \file Linear.h    \brief A linear function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_LINEAR_H
#define IMPCORE_LINEAR_H

#include "config.h"
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Linear function
/** \note The offset is not meaningful for optimization, but does
    make the displayed energies nicer.
 */
class Linear : public UnaryFunction
{
public:
  //! Create with the given offset and slope.
  Linear(Float offset, Float slope) : slope_(slope), offset_(offset) {}

  virtual ~Linear() {}

  virtual Float evaluate(Float feature) const {
    return (feature-offset_)*slope_;
  }

  virtual FloatPair evaluate_with_derivative(Float feature) const {
    return std::make_pair(evaluate(feature), slope_);
  }

  /** */
  void set_slope(Float f) {
    slope_=f;
  }

  /** */
  void set_offset(Float f) {
    offset_=f;
  }

  void show(std::ostream &out=std::cout) const {
    out << "Linear: " << slope_ << ", " << offset_ << std::endl;
  }

private:
  Float slope_, offset_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_LINEAR_H */

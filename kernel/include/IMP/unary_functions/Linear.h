/**
 *  \file Linear.h    \brief A linear function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_LINEAR_H
#define __IMP_LINEAR_H

#include "../UnaryFunction.h"

namespace IMP
{

//! Linear function
/** \ingroup unaryf
    \note The offset is not meaningful for optimization, but does
    make the displayed energies nicer.
 */
class Linear : public UnaryFunction
{
public:
  Linear(Float offset, Float slope) : slope_(slope), offset_(offset) {}

  virtual ~Linear() {}

  virtual Float evaluate(Float feature) { return (feature-offset_)*slope_; }

  virtual Float evaluate_deriv(Float feature, Float& deriv) {
    deriv= slope_;
    return evaluate(feature);
  }

  void set_slope(Float f) {
    slope_=f;
  }

  void set_offset(Float f) {
    offset_=f;
  }

  void show(std::ostream &out=std::cout) const {
    out << "Linear: " << slope_ << ", " << offset_ << std::endl;
  }

protected:
  Float slope_, offset_;
};

} // namespace IMP

#endif  /* __IMP_LINEAR_H */

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
    \note there is no offset since that is not meaningful for optimization
 */
class IMPDLLEXPORT Linear : public UnaryFunction
{
public:
  Linear(Float slope) : slope_(slope) {}

  virtual ~Linear() {}

  virtual Float operator()(Float feature) { return feature*slope_;}

  virtual Float operator()(Float feature, Float& deriv) {
    deriv= slope_;
    return operator()(feature);
  }

  void show(std::ostream &out=std::cout) const {
    out << "Linear: " << slope_ << std::endl;
  }

protected:
  Float slope_;
};

} // namespace IMP

#endif  /* __IMP_LINEAR_H */

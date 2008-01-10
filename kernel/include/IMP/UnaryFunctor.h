/**
 *  \file UnaryFunctor.h    \brief Single variable functor.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_UNARY_FUNCTOR_H
#define __IMP_UNARY_FUNCTOR_H

#include "IMP_config.h"
#include "base_types.h"
#include "Object.h"

namespace IMP
{

//! Abstract single variable functor class for score functions.
/** These functors take a single feature value, and return a corresponding
    score (and optionally also the first derivative).
 */
class IMPDLLEXPORT UnaryFunctor : public Object
{
public:
  UnaryFunctor() {}
  virtual ~UnaryFunctor() {}
  virtual Float operator()(Float feature, Float& deriv) = 0;
  virtual Float operator()(Float feature) = 0;
  virtual void show(std::ostream &out=std::cout) const = 0;
};

} // namespace IMP

#endif  /* __IMP_UNARY_FUNCTOR_H */

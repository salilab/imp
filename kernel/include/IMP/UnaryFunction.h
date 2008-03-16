/**
 *  \file UnaryFunction.h    \brief Single variable function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_UNARY_FUNCTION_H
#define __IMP_UNARY_FUNCTION_H

#include "IMP_config.h"
#include "base_types.h"
#include "internal/Object.h"

namespace IMP
{

//! Abstract single variable functor class for score functions.
/** These functors take a single feature value, and return a corresponding
    score (and optionally also the first derivative).
 */
class IMPDLLEXPORT UnaryFunction : public internal::Object
{
public:
  UnaryFunction() {}
  virtual ~UnaryFunction() {}

  //! Calculate score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float evaluate(Float feature) = 0;

  //! Calculate score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to the
                        given feaure.
      \return Score
   */
  virtual Float evaluate_deriv(Float feature, Float& deriv) = 0;

  virtual void show(std::ostream &out=std::cout) const = 0;
};

} // namespace IMP

#endif  /* __IMP_UNARY_FUNCTION_H */

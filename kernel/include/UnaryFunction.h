/**
 *  \file UnaryFunction.h    \brief Single variable function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMP_UNARY_FUNCTION_H
#define IMP_UNARY_FUNCTION_H

#include "config.h"
#include "base_types.h"
#include "RefCountedObject.h"
#include "Pointer.h"

IMP_BEGIN_NAMESPACE

//! Abstract single variable functor class for score functions.
/** These functors take a single feature value, and return a corresponding
    score (and optionally also the first derivative).
 */
class IMPEXPORT UnaryFunction : public RefCountedObject
{
public:
  UnaryFunction() {}

  //! Calculate score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual Float evaluate(Float feature) const = 0;

  //! Calculate score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return a FloatPair containing the score and its partial derivative
              with respect to the given feaure.
   */
  virtual FloatPair evaluate_with_derivative(Float feature) const = 0;

  //! Print information about the UnaryFunction to a stream.
  /** Should end in a newline.
    */
  virtual void show(std::ostream &out=std::cout) const = 0;

  IMP_REF_COUNTED_DESTRUCTOR(UnaryFunction)
};

IMP_OUTPUT_OPERATOR(UnaryFunction);

IMP_END_NAMESPACE

#endif  /* IMP_UNARY_FUNCTION_H */

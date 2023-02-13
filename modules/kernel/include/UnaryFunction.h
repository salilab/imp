/**
 *  \file IMP/UnaryFunction.h    \brief Single variable function.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_UNARY_FUNCTION_H
#define IMPKERNEL_UNARY_FUNCTION_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include <IMP/Object.h>
#include <boost/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

IMPKERNEL_BEGIN_NAMESPACE

//! Abstract single variable functor class for score functions.
/** These functors take a single feature value, and return a corresponding
    score (and optionally also the first derivative).

    Implementers should implement two functions:
    - virtual double evaluate(double feature) const
    - virtual DerivativePair evaluate_with_derivative(double feature) const
    also add IMP_OBJECT_METHODS(Name) macro for Object methods
 */
class IMPKERNELEXPORT UnaryFunction : public IMP::Object {
 public:
  UnaryFunction(std::string name = "UnaryFunction%1%");

  //! Calculate score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return Score
   */
  virtual double evaluate(double feature) const
#ifdef SWIG
      = 0;
#else
  {
    // to support easy generic classes
    return evaluate(feature);
  }
#endif

  //! Calculate score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \return a DerivativePair containing the score and its partial derivative
              with respect to the given feature.
   */
  virtual DerivativePair evaluate_with_derivative(double feature) const {
    // to support easy generic classes
    return evaluate_with_derivative(feature);
  }

  IMP_REF_COUNTED_DESTRUCTOR(UnaryFunction);

private:
  friend class boost::serialization::access;

#if BOOST_VERSION >= 106500 && BOOST_VERSION <= 106600
  template<class T>
  friend struct boost::archive::detail::heap_allocation;
#endif

  template<class Archive> void serialize(Archive &ar, const unsigned int) {
    ar & boost::serialization::base_object<Object>(*this);
  }
};

IMP_OBJECTS(UnaryFunction, UnaryFunctions);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_UNARY_FUNCTION_H */

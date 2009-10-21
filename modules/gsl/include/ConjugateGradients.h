/**
 *  \file gsl/ConjugateGradients.h
 *  \brief A conjugate gradients optimizer from GSL
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPGSL_CONJUGATE_GRADIENTS_H
#define IMPGSL_CONJUGATE_GRADIENTS_H

#include "config.h"

#include "GSLOptimizer.h"

IMPGSL_BEGIN_NAMESPACE

//! A conjugate gradients optimizer taken from GSL
/** \see IMP::core::ConjugateGradients
 */
class IMPGSLEXPORT ConjugateGradients: public GSLOptimizer
{
  double initial_step_, line_step_, min_gradient_;
public:
  //!
  ConjugateGradients();

  //! Set the gradient threshold
  void set_threshold(double mg) {
    min_gradient_=mg;
  }

  IMP_OPTIMIZER(ConjugateGradients, get_module_version_info())
};


IMPGSL_END_NAMESPACE

#endif  /* IMPGSL_CONJUGATE_GRADIENTS_H */

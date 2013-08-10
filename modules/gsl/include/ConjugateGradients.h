/**
 *  \file IMP/gsl/ConjugateGradients.h
 *  \brief A conjugate gradients optimizer from GSL
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPGSL_CONJUGATE_GRADIENTS_H
#define IMPGSL_CONJUGATE_GRADIENTS_H

#include <IMP/gsl/gsl_config.h>

#include "GSLOptimizer.h"

IMPGSL_BEGIN_NAMESPACE

//! A conjugate gradients optimizer taken from GSL
/** \see IMP::core::ConjugateGradients
 */
class IMPGSLEXPORT ConjugateGradients : public GSLOptimizer {
  double initial_step_, line_step_, min_gradient_;

 public:
  //!
  ConjugateGradients(Model *m);
  /** \deprecated_at{2.1} Pass a model to the constructor. */
  IMPGSL_DEPRECATED_FUNCTION_DECL(2.1)
  ConjugateGradients();

  //! Set the gradient threshold
  void set_threshold(double mg) { min_gradient_ = mg; }
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConjugateGradients);
};

IMPGSL_END_NAMESPACE

#endif /* IMPGSL_CONJUGATE_GRADIENTS_H */

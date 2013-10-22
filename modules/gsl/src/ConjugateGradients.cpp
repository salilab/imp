/**
 *  \file ConjugateGradients.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/gsl/ConjugateGradients.h"
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_NAMESPACE

ConjugateGradients::ConjugateGradients(kernel::Model *m) : GSLOptimizer(m) {
  initial_step_ = .01;
  line_step_ = .0001;
  min_gradient_ = .001;
}

ConjugateGradients::ConjugateGradients() : GSLOptimizer() {
  IMPGSL_DEPRECATED_FUNCTION_DEF(2.1, "Pass the model to the constructor.");
  initial_step_ = .01;
  line_step_ = .0001;
  min_gradient_ = .001;
}

Float ConjugateGradients::do_optimize(unsigned int nsteps) {
  const gsl_multimin_fdfminimizer_type *t =
      gsl_multimin_fdfminimizer_conjugate_fr;
  return GSLOptimizer::optimize(nsteps, t, initial_step_, line_step_,
                                min_gradient_);
}

IMPGSL_END_NAMESPACE

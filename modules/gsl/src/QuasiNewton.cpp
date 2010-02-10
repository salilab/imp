/**
 *  \file QuasiNewton.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/gsl/QuasiNewton.h"
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_NAMESPACE

QuasiNewton::QuasiNewton(Model *m): GSLOptimizer(m) {
  initial_step_=.01;
  line_step_=.01;
  min_gradient_=.001;
}

void QuasiNewton::show(std::ostream &out) const {
  out << "QuasiNewton optimizer" << std::endl;
}

Float QuasiNewton::optimize(unsigned int nsteps) {
  const gsl_multimin_fdfminimizer_type *t
    =gsl_multimin_fdfminimizer_vector_bfgs2;
  return GSLOptimizer::optimize(nsteps, t, initial_step_,
                                line_step_, min_gradient_);
}

IMPGSL_END_NAMESPACE

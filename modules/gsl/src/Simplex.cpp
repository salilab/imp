/**
 *  \file Simplex.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/gsl/Simplex.h"
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_NAMESPACE

Simplex::Simplex(Model *m) : GSLOptimizer(m) {
  min_length_ = .1;
  max_length_ = 1;
}

void Simplex::do_show(std::ostream &) const {}

Float Simplex::do_optimize(unsigned int nsteps) {
  // we have an old version of GSL so can't use the 2 version
  const gsl_multimin_fminimizer_type *t = gsl_multimin_fminimizer_nmsimplex;
  return GSLOptimizer::optimize(nsteps, t, min_length_, max_length_);
}

IMPGSL_END_NAMESPACE

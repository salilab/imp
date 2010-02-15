/**
 *  \file Simplex.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/gsl/Simplex.h"
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_NAMESPACE

Simplex::Simplex(Model *m): GSLOptimizer(m) {
  min_length_=.1;
  max_length_=100;
}


void Simplex::do_show(std::ostream &out) const {}

Float Simplex::optimize(unsigned int nsteps) {
  // we have an old version of GSL so can't use the 2 version
  const gsl_multimin_fminimizer_type *t=gsl_multimin_fminimizer_nmsimplex;
  return GSLOptimizer::optimize(100, t, min_length_, max_length_);
}

IMPGSL_END_NAMESPACE

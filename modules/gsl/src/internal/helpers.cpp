/**
 *  \file gsl/internal/helpers.cpp
 *  \brief Helpers for using IMP with GSL
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/gsl/GSLOptimizer.h"
#include "IMP/gsl/internal/helpers.h"
#include <gsl/gsl_vector.h>

IMPGSL_BEGIN_INTERNAL_NAMESPACE
namespace {
double gsl_f(const gsl_vector *v, void *data) {
  const GSLOptimizer *copt = reinterpret_cast<const GSLOptimizer *>(data);
  GSLOptimizer *opt = const_cast<GSLOptimizer *>(copt);
  return opt->evaluate(v);
}

void gsl_df(const gsl_vector *v, void *data, gsl_vector *df) {
  const GSLOptimizer *copt = reinterpret_cast<const GSLOptimizer *>(data);
  GSLOptimizer *opt = const_cast<GSLOptimizer *>(copt);
  opt->evaluate_derivative(v, df);
}

void gsl_fdf(const gsl_vector *v, void *data, double *f, gsl_vector *df) {
  const GSLOptimizer *copt = reinterpret_cast<const GSLOptimizer *>(data);
  GSLOptimizer *opt = const_cast<GSLOptimizer *>(copt);
  *f = opt->evaluate_derivative(v, df);
}
}

gsl_multimin_function_fdf create_function_data(const GSLOptimizer *opt) {
  gsl_multimin_function_fdf ret;
  ret.n = opt->get_dimension();
  ret.f = &gsl_f;
  ret.df = &gsl_df;
  ret.fdf = &gsl_fdf;
  ret.params = const_cast<void *>(static_cast<const void *>(opt));
  return ret;
}

gsl_multimin_function create_f_function_data(const GSLOptimizer *opt) {
  gsl_multimin_function ret;
  ret.n = opt->get_dimension();
  ret.f = &gsl_f;
  ret.params = const_cast<void *>(static_cast<const void *>(opt));
  return ret;
}

IMPGSL_END_INTERNAL_NAMESPACE

/**
 *  \file gsl/internal/helpers.h
 *  \brief Helpers for using IMP with GSL
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPGSL_INTERNAL_HELPERS_H
#define IMPGSL_INTERNAL_HELPERS_H

#include <IMP/gsl/gsl_config.h>
#include "../GSLOptimizer.h"
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_INTERNAL_NAMESPACE

gsl_multimin_function_fdf create_function_data(const GSLOptimizer *opt);
gsl_multimin_function create_f_function_data(const GSLOptimizer *opt);

IMPGSL_END_INTERNAL_NAMESPACE

#endif /* IMPGSL_INTERNAL_HELPERS_H */

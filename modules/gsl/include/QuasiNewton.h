/**
 *  \file QuasiNewton.h
 *  \brief A GSL-based Quasi-Newton optimizer
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPGSL_QUASI_NEWTON_H
#define IMPGSL_QUASI_NEWTON_H

#include "config.h"

#include "GSLOptimizer.h"

IMPGSL_BEGIN_NAMESPACE

//! A quasi-Newton optimizer taken from GSL
/** \untested{QuasiNewton}
 */
class IMPGSLEXPORT QuasiNewton: public GSLOptimizer
{
  double initial_step_, line_step_, min_gradient_;
public:
  QuasiNewton(Model *m=NULL);

  IMP_OPTIMIZER(QuasiNewton, get_module_version_info())
};


IMPGSL_END_NAMESPACE

#endif  /* IMPGSL_QUASI_NEWTON_H */

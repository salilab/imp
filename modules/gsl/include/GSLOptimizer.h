/**
 *  \file GSLOptimizer.h
 *  \brief A base class for GSL-based optimizers
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPGSL_GSL_OPTIMIZER_H
#define IMPGSL_GSL_OPTIMIZER_H

#include "config.h"

#include <IMP/Optimizer.h>
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_NAMESPACE

//! A base class for GSL-based optimizers
/** This class is a base class for the various GSL-based optimizers. It exposes
    very little user functionality.
 */
class IMPGSLEXPORT GSLOptimizer: public Optimizer
{
  double stop_score_;
  mutable double best_score_;
public:
  GSLOptimizer(Model *m);

  virtual ~GSLOptimizer();

  using Optimizer::optimize;

  //! Stop the optimization if the score falls below this value
  void set_stop_score(double d) {
    stop_score_=d;
  }
#ifndef IMP_DOXYGEN
  gsl_vector* get_state() const;
  void update_state(gsl_vector *x) const;
  void write_state(const gsl_vector *x) const;

  double evaluate(const gsl_vector *v) const;
  double evaluate_derivative(const gsl_vector *v, gsl_vector *df) const;
  unsigned int get_dimension() const {
    return std::distance(float_indexes_begin(), float_indexes_end());
  }
  double optimize(unsigned int n, const gsl_multimin_fdfminimizer_type*t,
                  double step, double param, double min_gradient) const;

  double optimize(unsigned int n, const gsl_multimin_fminimizer_type*t,
                  double size, double max_size) const;
#endif
};


IMPGSL_END_NAMESPACE

#endif  /* IMPGSL_GSL_OPTIMIZER_H */

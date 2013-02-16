/**
 *  \file IMP/gsl/GSLOptimizer.h
 *  \brief A base class for GSL-based optimizers
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPGSL_GSL_OPTIMIZER_H
#define IMPGSL_GSL_OPTIMIZER_H

#include <IMP/gsl/gsl_config.h>

#include <IMP/Optimizer.h>
#include <IMP/optimizer_macros.h>
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
  mutable FloatIndexes fis_;
public:
  GSLOptimizer(Model *m);

  virtual ~GSLOptimizer();

#ifndef SWIG
  using Optimizer::optimize;
#endif

  //! Stop the optimization if the score falls below this value
  void set_stop_score(double d) {
    stop_score_=d;
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  gsl_vector* get_state() const;
  void update_state(gsl_vector *x) const;
  void write_state(const gsl_vector *x) const;

  double evaluate(const gsl_vector *v);
  double evaluate_derivative(const gsl_vector *v, gsl_vector *df);
  unsigned int get_dimension() const {
    IMP_USAGE_CHECK(!fis_.empty(), "not initialized properly");
    return fis_.size();
  }
  double optimize(unsigned int n, const gsl_multimin_fdfminimizer_type*t,
                  double step, double param, double min_gradient);

  double optimize(unsigned int n, const gsl_multimin_fminimizer_type*t,
                  double size, double max_size);
#endif
};
IMP_OBJECTS(GSLOptimizer, GSLOptimizers);


IMPGSL_END_NAMESPACE

#endif  /* IMPGSL_GSL_OPTIMIZER_H */

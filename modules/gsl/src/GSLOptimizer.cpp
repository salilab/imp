/**
 *  \file GSLOptimizer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/gsl/GSLOptimizer.h"
#include "IMP/gsl/internal/helpers.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

IMPGSL_BEGIN_NAMESPACE
namespace {
  struct AllDone{};
}


GSLOptimizer::GSLOptimizer( ):
  stop_score_(-std::numeric_limits<double>::max()),
  best_score_(std::numeric_limits<double>::max()){
}

GSLOptimizer::~GSLOptimizer(){}

gsl_vector* GSLOptimizer::get_state() const {
  gsl_vector *r= gsl_vector_alloc (get_dimension());
  int i=0;
  for (FloatIndexIterator it= float_indexes_begin();
       it != float_indexes_end(); ++it) {
    gsl_vector_set(r, i, get_scaled_value(*it));
    ++i;
  }
  return r;
}

void GSLOptimizer::update_state(gsl_vector*x) const {
  int i=0;
  for (FloatIndexIterator it= float_indexes_begin();
       it != float_indexes_end(); ++it) {
    gsl_vector_set(x, i, get_scaled_value(*it));
    ++i;
  }
}

void GSLOptimizer::write_state(const gsl_vector*x) const {
  int i=0;
  for (FloatIndexIterator it= float_indexes_begin();
       it != float_indexes_end(); ++it) {
    set_scaled_value(*it, gsl_vector_get(x, i));
    ++i;
  }
}


double GSLOptimizer::evaluate(const gsl_vector *v) const {
  /* set model state */
  write_state(v);
  /* get score */
  double score= get_model()->evaluate(false);
  best_score_=std::min(score, best_score_);
  if (score < stop_score_) {
    throw AllDone();
  }
  return score;
}

double GSLOptimizer::evaluate_derivative(const gsl_vector *v,
                                         gsl_vector *df) const {
   /* set model state */
  write_state(v);

  /* get score */
  double score= get_model()->evaluate(true);
  best_score_=std::min(score, best_score_);
  if (score < stop_score_) {
    throw AllDone();
  }
  /* get derivatives */
  {
    int i=0;
    for (FloatIndexIterator it= float_indexes_begin();
         it != float_indexes_end(); ++it) {
      double d= get_scaled_derivative(*it);
      gsl_vector_set(df, i, d);
      ++i;
    }
  }
  return score;
}

double GSLOptimizer::optimize(unsigned int iter,
                              const gsl_multimin_fdfminimizer_type*t,
                              double step, double param,
                              double min_gradient) const {
  best_score_=std::numeric_limits<double>::max();
  unsigned int n= get_dimension();
  if (n ==0) {
    IMP_LOG(TERSE, "Nothing to optimize" << std::endl);
    return get_model()->evaluate(false);
  }
  gsl_multimin_fdfminimizer *s=gsl_multimin_fdfminimizer_alloc (t, n);

  gsl_vector *x= gsl_vector_alloc(get_dimension());

  gsl_multimin_function_fdf f= internal::create_function_data(this);

  update_state(x);
  gsl_multimin_fdfminimizer_set (s, &f, x, step, param);

  try {
    int status;
    do {
      --iter;
      //update_state(x);
      status = gsl_multimin_fdfminimizer_iterate(s);
      update_states();
      if (status) {
        IMP_LOG(TERSE, "Ending optimization because of status "
                << status << std::endl);
        break;
      }
      status = gsl_multimin_test_gradient (s->gradient, min_gradient);
      if (status == GSL_SUCCESS) {
        IMP_LOG(TERSE, "Ending optimization because of small gradient "
                << s->gradient << std::endl);
        break;
      }
    } while (status == GSL_CONTINUE && iter >0);
  } catch (AllDone){
  }
  gsl_vector *ret=gsl_multimin_fdfminimizer_x (s);
  write_state(ret);
  gsl_multimin_fdfminimizer_free (s);
  gsl_vector_free (x);
  return best_score_;
}


double GSLOptimizer::optimize(unsigned int iter,
                              const gsl_multimin_fminimizer_type*t,
                              double ms, double mxs) const {
  best_score_=std::numeric_limits<double>::max();
  unsigned int n= get_dimension();
  if (n ==0) {
    IMP_LOG(TERSE, "Nothing to optimize" << std::endl);
    return get_model()->evaluate(false);
  }
  gsl_multimin_fminimizer *s=gsl_multimin_fminimizer_alloc (t, n);

  gsl_vector *x= gsl_vector_alloc(get_dimension());
  update_state(x);
  gsl_vector *ss= gsl_vector_alloc(get_dimension());
  gsl_vector_set_all(ss, mxs);

  gsl_multimin_function f= internal::create_f_function_data(this);
  gsl_multimin_fminimizer_set (s, &f, x, ss);
  try {
    int status;
    do {
      --iter;
      //update_state(x);
      status = gsl_multimin_fminimizer_iterate(s);
      if (status) {
        IMP_LOG(TERSE, "Ending optimization because of state " << s
                << std::endl);
        break;
      }
      double sz= gsl_multimin_fminimizer_size(s);
      status= gsl_multimin_test_size(sz, ms);
      update_states();
      if (status == GSL_SUCCESS) {
        IMP_LOG(TERSE, "Ending optimization because of small size " << sz
                << std::endl);
        break;
      }
    } while (status == GSL_CONTINUE && iter >0);
  } catch (AllDone){
  }
  gsl_vector *ret=gsl_multimin_fminimizer_x (s);
  best_score_=gsl_multimin_fminimizer_minimum (s);
  write_state(ret);
  gsl_multimin_fminimizer_free (s);
  gsl_vector_free (x);
  return best_score_;
}



IMPGSL_END_NAMESPACE

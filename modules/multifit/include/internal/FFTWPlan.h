/**
 *  \file FFTWPlan.h   \brief C++ smart pointer to FFTW plans.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FFTW_PLAN_H
#define IMPMULTIFIT_FFTW_PLAN_H

#include <IMP/base_types.h>
#include <IMP/log.h>
#include <IMP/exception.h>
#include <IMP/multifit/multifit_config.h>
#include "fftw3.h"
#include <boost/noncopyable.hpp>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

class IMPMULTIFITEXPORT FFTWPlan : public boost::noncopyable
{
private:
  fftw_plan data_;
  static unsigned int number_of_plans_;
  static bool cleanup_requested_;

  static void check_cleanup() {
    if (number_of_plans_ == 0 && cleanup_requested_) {
      // Must not call fftw_cleanup while plans still exist, as it causes
      // existing plans to become undefined. Also, fftw_cleanup() throws away
      // accumulated FFTW wisdom, so keep it around until a cleanup has been
      // specificially requested (i.e. we're done with FFTW)
      IMP_LOG_VERBOSE( "Doing FFTW cleanup");
      fftw_cleanup();
    }
  }

  void set_pointer(fftw_plan plan) {
    release();
    data_ = plan;
    if (plan) {
      ++number_of_plans_;
    }
  }

public:
  FFTWPlan() : data_(nullptr) {}

  FFTWPlan(fftw_plan plan) : data_(nullptr) { set_pointer(plan); }

  FFTWPlan &operator=(fftw_plan plan) {
    set_pointer(plan);
    return *this;
  }

  ~FFTWPlan() {
    release();
  }

  void release() {
    if (data_) {
      fftw_destroy_plan(data_);
      IMP_INTERNAL_CHECK(number_of_plans_ >= 1, "Inconsistent number of plans");
      --number_of_plans_;
      check_cleanup();
    }
  }

  static void request_cleanup() {
    cleanup_requested_ = true;
    check_cleanup();
  }

  // Get the raw pointer
  fftw_plan get() const { return data_; }
};

IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif  /* IMPMULTIFIT_FFTW_PLAN_H */

/**
 *  \file FFTWPlan.cpp   \brief C++ smart pointer to FFTW plans.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/internal/FFTWPlan.h>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

unsigned int FFTWPlan::number_of_plans_ = 0;
bool FFTWPlan::cleanup_requested_ = false;

IMPMULTIFIT_END_INTERNAL_NAMESPACE

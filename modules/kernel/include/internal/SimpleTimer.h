/**
 *  \file internal/SimpleTimer.h
 *  \brief Simple subsecond timer for benchmarking or logging
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SIMPLE_TIMER_H
#define IMPKERNEL_INTERNAL_SIMPLE_TIMER_H

#include <IMP/kernel_config.h>
#include <chrono>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

class SimpleTimer {
  std::chrono::steady_clock::time_point start_time_;
public:
  SimpleTimer() {
    restart();
  }

  void restart() {
    start_time_ = std::chrono::steady_clock::now();
  }

  double elapsed() const {
    auto end_time = std::chrono::steady_clock::now();
    auto time_span = std::chrono::duration_cast<
                      std::chrono::duration<double> >(end_time - start_time_);
    return time_span.count();
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_SIMPLE_TIMER_H */

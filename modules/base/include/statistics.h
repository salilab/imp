/**
 *  \file IMP/base/statistics.h
 *  \brief Manage statistics on IMP runs.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_STATISTICS_H
#define IMPBASE_STATISTICS_H

#include <IMP/base/base_config.h>
#include "file.h"
#include "internal/static.h"
#include <string>
#include <boost/timer.hpp>

IMPBASE_BEGIN_NAMESPACE

/** Reset all the statistics for IMP. */
IMPBASEEXPORT void clear_statistics();

/** Show all captured timings. */
IMPBASEEXPORT void show_timings(TextOutput out);

/** Time an operation and save the timings.*/
class Timer: public RAII {
  boost::timer timer_;
  std::string key_;
 public:
  Timer(const Object *object, std::string operation) {
    key_ = object->get_name() + "::" + operation;
  }
  Timer(std::string operation) {
    key_ = operation;
  }
  ~Timer() {
    internal::timings[key_].total_time += timer_.elapsed();
    ++internal::timings[key_].calls;
  }
};

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_STATISTICS_H */

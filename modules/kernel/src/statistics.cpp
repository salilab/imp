/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base_statistics.h"
#include "internal/base_static.h"
#include <boost/format.hpp>

IMPKERNEL_BEGIN_NAMESPACE

void Timer::initialize(std::string operation) {
  start_ = std::clock();
  key_ = operation;
}
void Timer::save() {
  // from boost/timer.hpp
  internal::timings[key_].total_time +=
      double(std::clock() - start_) / CLOCKS_PER_SEC;
  ++internal::timings[key_].calls;
}

void clear_statistics() {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.7, "Use profiling tools instead");
  internal::timings.clear();
}

void show_timings(TextOutput out) {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.7, "Use profiling tools instead");
  out.get_stream() << (boost::format("%-60s%10s%8s") % "Operation," %
                       "seconds," % "calls,") << std::endl;
  typedef std::pair<std::string, internal::Timing> VT;
  IMP_FOREACH(VT tp, internal::timings) {
    std::string name = tp.first;
    if (name.size() > 60) {
      name = std::string(name.begin(), name.begin() + 60);
    }
    out.get_stream() << (boost::format("%-61s%10f,%8d") % (name + ",") %
                         tp.second.total_time % tp.second.calls) << std::endl;
  }
}

void set_statistics_level(StatisticsLevel l) {
  IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.7, "Use profiling tools instead");
  internal::stats_level = l;
}

IMPKERNEL_END_NAMESPACE

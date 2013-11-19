/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/statistics.h"
#include "IMP/base/internal/static.h"
#include <boost/format.hpp>

IMPBASE_BEGIN_NAMESPACE

Timer::Timer(const Object *object, std::string operation) {
  key_ = object->get_name() + "::" + operation;
}
Timer::Timer(std::string operation) { key_ = operation; }
Timer::~Timer() {
  internal::timings[key_].total_time += timer_.elapsed();
  ++internal::timings[key_].calls;
}

void clear_statistics() { internal::timings.clear(); }

void show_timings(TextOutput out) {
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

IMPBASE_END_NAMESPACE

/**
 *  \file IMP/base_statistics.h
 *  \brief Manage statistics on IMP runs.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_BASE_STATISTICS_H
#define IMPKERNEL_BASE_STATISTICS_H

#include <IMP/kernel_config.h>
#include "file.h"
#include "enums.h"
#include "internal/base_static.h"
#include <string>
#include <ctime>

IMPKERNEL_BEGIN_NAMESPACE

IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.7)
IMPKERNELEXPORT void clear_statistics();

IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.7)
IMPKERNELEXPORT void show_timings(TextOutput out);

//! Time an operation and save the timings.
class IMPKERNELEXPORT Timer : public RAII {
  std::clock_t start_;
  std::string key_;

  void initialize(std::string key);
  void save();

 public:
  // TODO: string ops may be wasteful - could pass pointer
  //       to const char* or reference to stringsto be initialized?
  Timer(const Object *object, std::string const& operation) {
    if (internal::stats_level > NO_STATISTICS) {
      initialize(object->get_name() + "::" + operation);
    }
  }
  // TODO: string ops may be wasteful - could pass pointer
  //       to const char* or reference to stringsto be initialized?
  Timer(const Object *object, const char* operation) {
    if (internal::stats_level > NO_STATISTICS) {
      std::string s_operation(operation);
      initialize(object->get_name() + "::" + operation);
    }
  }

  Timer(std::string operation) {
    if (internal::stats_level > NO_STATISTICS) {
      initialize(operation);
    }
  }
  ~Timer() {
    if (!key_.empty()) save();
  }
};

IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.7)
IMPKERNELEXPORT void set_statistics_level(StatisticsLevel l);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_BASE_STATISTICS_H */

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
#include "enums.h"
#include "internal/static.h"
#include <string>
#include <ctime>

IMPBASE_BEGIN_NAMESPACE

/** Reset all the statistics for IMP. */
IMPBASEEXPORT void clear_statistics();

/** Show all captured timings. */
IMPBASEEXPORT void show_timings(TextOutput out);

/** Time an operation and save the timings.*/
class IMPBASEEXPORT Timer : public RAII {
  std::clock_t start_;
  std::string key_;

  void initialize(std::string key);
  void save();
 public:
  Timer(const Object *object, std::string operation) {
    if (internal::stats_level > NO_STATISTICS) {
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

/** Set the level of statistics to be gathered. */
IMPBASEEXPORT void set_statistics_level(StatisticsLevel l);

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_STATISTICS_H */

/** \file utility.cpp Benchmarking utilities
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/utility.h>
#include <IMP/base/exception.h>
#include <IMP/base/log_macros.h>
#include <iostream>
#include <boost/format.hpp>
#include <algorithm>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/log.h>
IMPBENCHMARK_BEGIN_NAMESPACE

void report(std::string name, std::string algorithm, double value,
            double check) {
  if (value < 0) {
    // disabled
  } else {
    std::cout << boost::format("%s, %s, %30t%.2e, %50t%1.1e, %d") % name %
                     algorithm % value % check %
                     (internal::current_benchmark) << std::endl;
  }
}
void report(std::string name, double value, double check) {
  report(name, "", value, check);
}

int get_return_value() { return 0; }

IMPBENCHMARK_END_NAMESPACE

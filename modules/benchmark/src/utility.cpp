/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/utility.h>
#include <IMP/exception.h>
#include <iostream>
#include <boost/format.hpp>
#include <algorithm>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/log.h>
IMPBENCHMARK_BEGIN_NAMESPACE



void report(std::string name, double value, double check) {
  if (value < 0) {
    IMP_WARN("Negative value passed: " << value << std::endl);
    value=0;
  }
  std::cout << boost::format("%s, %30t%.2e, %50t%1.1e")
    %name % value % check << std::endl;
}


int get_return_value() {
  return 0;
}

IMPBENCHMARK_END_NAMESPACE

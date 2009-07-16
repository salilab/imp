/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/benchmark/utility.h>
#include <iostream>
#include <boost/format.hpp>
#include <IMP/log.h>
IMPBENCHMARK_BEGIN_NAMESPACE

void report(std::string name, double value, double check) {
  if (value < 0) {
    IMP_WARN("Negative value passed: " << value);
    value=0;
  }
  std::cout << boost::format("%s,%f,%e\n")%name % value % check;
}


IMPBENCHMARK_END_NAMESPACE

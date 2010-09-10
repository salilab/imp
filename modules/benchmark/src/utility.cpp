/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
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
  std::cout << boost::format("%s,%f,%e\n")%name % value % check;
}

std::pair<double, double> get_baseline() {
  double timev;
  double ret=0;
  IMP_TIME({std::vector<int> ints;
      for (unsigned int i=0; i< 100000; ++i) {
        ints.push_back(i);
      }
      std::random_shuffle(ints.begin(), ints.end());
      std::sort(ints.begin(), ints.end());
      for (unsigned int i=0; i< ints.size(); ++i) {
        ret+= ints[i];
      }
    }, timev);
  return std::make_pair(timev, ret);
}


IMPBENCHMARK_END_NAMESPACE

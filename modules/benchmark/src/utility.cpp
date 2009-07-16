/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/benchmark/utility.h>
#include <iostream>

IMPBENCHMARK_BEGIN_NAMESPACE

void report(std::string name, double value, double check) {
  std::cout << name << ", " << value << ", " << check << std::endl;
}


IMPBENCHMARK_END_NAMESPACE

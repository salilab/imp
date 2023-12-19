/** \file utility.cpp Benchmarking utilities
 *
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/internal/control.h>
#include <IMP/internal/directories.h>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

int current_benchmark = -1;

std::string get_file_name(std::string suffix) {
  std::string exe = IMP::get_executable_name();
  std::string exebase = IMP::internal::get_file_name(exe);
  std::ostringstream oss;
  oss << exebase << current_benchmark << suffix;
  return oss.str();
}
IMPBENCHMARK_END_INTERNAL_NAMESPACE

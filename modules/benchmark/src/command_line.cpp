/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/command_line.h>
IMPBENCHMARK_BEGIN_NAMESPACE

bool help=false;
bool profile_benchmark=false;
bool profile_all=false;
bool heap_profile_benchmark=false;
bool heap_profile_all=false;
bool leak_check_benchmark=false;
bool leak_check_all=false;
int run_only=-1;
int next_benchmark=0;
int log_level=0;
std::string benchmarks_name;
IMPBENCHMARK_END_NAMESPACE

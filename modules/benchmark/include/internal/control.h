/**
 *  \file benchmark/control.h
 *  \brief Various utilities for benchmarking
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_INTERNAL_CONTROL_H
#define IMPBENCHMARK_INTERNAL_CONTROL_H

#include <IMP/benchmark/benchmark_config.h>
#include <IMP/base/flags.h>
#include <string>

IMPBENCHMARK_BEGIN_INTERNAL_NAMESPACE

extern IMPBENCHMARKEXPORT int current_benchmark;

IMPBENCHMARKEXPORT std::string get_file_name(std::string suffix);

IMPBENCHMARK_END_INTERNAL_NAMESPACE

#endif  /* IMPBENCHMARK_INTERNAL_CONTROL_H */

/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/profile.h>
#ifdef IMP_BENCHMARK_USE_GOOGLE_PERFTOOLS_PROFILE
#include <google/profiler.h>
#endif

#include <boost/format.hpp>
#include <IMP/base/check_macros.h>

IMPBENCHMARK_BEGIN_NAMESPACE

#ifdef IMP_BENCHMARK_USE_GOOGLE_PERFTOOLS_PROFILE
namespace {
std::string profname="imp.%1%.prof";
int last_prof=-1;
}

void set_profile_name(std::string name) {
  profname=name;
  last_prof=-1;
}

void set_is_profiling(bool tf) {
  if (tf) {
    ++last_prof;
    std::string name;
    if (std::find(profname.begin(), profname.end(), '%') != profname.end()) {
      std::ostringstream oss;
      try {
        oss << boost::format(profname)
            % last_prof;
        name=oss.str();
      } catch(...) {
        IMP_THROW("Invalid format specified in profile name, should be %1%: "
                  << profname,
                  ValueException);
      }
    } else {
      name=profname;
    }
    ProfilerStart(name.c_str());
  } else {
    ProfilerStop();
  }
}
#endif

IMPBENCHMARK_END_NAMESPACE

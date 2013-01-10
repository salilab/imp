/** \file utility.cpp Benchmarking utilties
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <RMF/profile.h>
#if defined(RMF_BENCHMARK_USE_GPERFTOOLS)
#  include <gperftools/profiler.h>
#elif defined(RMF_BENCHMARK_USE_GOOGLEPERFTOOLS)
#  include <google/profiler.h>
#endif

#include <boost/format.hpp>
#include <RMF/infrastructure_macros.h>
#include <sstream>

namespace RMF {
namespace {
std::string profname = "RMF.%1%.prof";
int last_prof = -1;
}

void set_profile_name(std::string name) {
  profname = name;
  last_prof = -1;
}

#if defined(RMF_BENCHMARK_USE_GPERFTOOLS) \
  || defined(RMF_BENCHMARK_USE_GOOGLEPERFTOOLS)
void set_is_profiling(bool tf) {
  if (tf) {
    ++last_prof;
    std::string name;
    if (std::find(profname.begin(), profname.end(), '%') != profname.end()) {
      std::ostringstream oss;
      try {
        oss << boost::format(profname)
          % last_prof;
        name = oss.str();
      } catch(...) {
        RMF_THROW("Invalid format specified in profile name, should be %1%",
                  UsageException);
      }
    } else {
      name = profname;
    }
    ProfilerStart(name.c_str());
  } else {
    ProfilerStop();
  }
}
#else
void set_is_profiling(bool tf) {
  if (tf) {
    std::cerr << "Google Prof Tools were not found, no profiling available,\n";
  }
}
#endif

}

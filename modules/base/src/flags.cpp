/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/flags.h>
#include <IMP/base/file.h>
#include <IMP/base/types.h>
#include <IMP/base/log_macros.h>
#if IMP_BASE_HAS_GPERFTOOLS
#include <gperftools/profiler.h>
#endif
#if MP_BASE_HAS_TCMALLOC_HEAPPROFILER
#include <gperftools/heap-profiler.h>
#endif

IMPBASE_BEGIN_NAMESPACE

static const char *check_help=
  "The level of checking to use: 0 for NONE, 1 for USAGE and 2 for ALL.";

#if IMP_BUILD == IMP_FAST
 IMP_DEFINE_INT(check_level, NONE, check_help);
#elif IMP_BUILD == IMP_RELEASE
  IMP_DEFINE_INT(check_level, USAGE, check_help);
#else
  IMP_DEFINE_INT(check_level, USAGE_AND_INTERNAL, check_help);
#endif
 IMP_DEFINE_INT(log_level, TERSE,
         "The log level, 0 for NONE, 1 for WARN, 2 for TERSE, 3 for VERBOSE");

#if IMP_BASE_HAS_GPERFTOOLS
IMP_DEFINE_BOOL(cpu_profile, false, "Perform CPU profiling.");
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
IMP_DEFINE_BOOL(heap_profile, false, "Perform heap profiling.");
#endif

namespace {
  std::string exename;

  void initialize(std::string path) {
    std::string exename= internal::get_file_name(path);
#if IMP_BASE_HAS_GPERFTOOLS
    if (FLAGS_cpu_profile) {
      std::string name=exename+".pprof";
      ProfilerStart(name.c_str());
    }
#endif
#if IMP_BASE_HAS_HEAPPROFILER
    if (FLAGS_heap_profile) {
      std::string name=exename+".hprof";
      HeapProfilerStart(name.c_str());
    }
#endif
  }
}

std::string get_executable_name() {
  return exename;
}
#if IMP_BASE_HAS_GFLAGS

base::Vector<std::string> setup_from_argv(int argc, char **argv,
                         int num_positional) {
   exename= argv[0];
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (num_positional != -1 && num_positional != argc-1) {
    google::ShowUsageWithFlags(exename.c_str());
    exit(1);
  }
  Strings ret;
  for (unsigned int i=0; i< argc; ++i) {
    ret.push_back(std::string(argv[i]));
  }
  initialize(exename);
  return ret;
}

base::Vector<std::string> setup_from_argv(Strings iargv,
                        int num_positional) {
  // hack for now, leaks
  char **argv= new char*[iargv.size()];
  for (unsigned int i=0; i < iargv.size(); ++i) {
    argv[i]= new char[iargv[i].size()+1];
    std::copy(iargv[i].begin(), iargv[i].end(), argv[i]);
    argv[i][iargv[i].size()]='\0';
  }
  return setup_from_argv(iargv.size(), argv, num_positional);
}
#else

Strings setup_from_argv(int argc, char **argv,
                        int num_positional) {
  Strings ret;
  exename=argv[0];
  for ( int i=1; i< argc; ++i) {
    if (argv[i][0] != '-') {
      ret.push_back(std::string(argv[i]));
    } else {
      IMP_LOG(WARNING, "Command line arguments requires gflags."
              << "Ignoring " << argv[i]);
    }
  }
  initialize(argv[0]);
  return ret;
}

Strings setup_from_argv(Strings argv,
                        int num_positional) {
  Strings ret;
  for (unsigned int i=1; i< argv.size(); ++i) {
    if (argv[i][0] != '-') {
      ret.push_back(argv[i]);
    } else {
      IMP_LOG(WARNING, "Command line arguments requires gflags."
              << "Ignoring " << argv[i]);
    }
  }
  return ret;
}
#endif

#if IMP_BASE_HAS_GFLAGS

namespace {
  struct FlagData {
    boost::shared_ptr<google::FlagRegisterer> registerer;
    fLS::clstring value;
    fLS::clstring default_value;
  };
  compatibility::map<std::string, FlagData> flag_data;
}

void add_string_flag(std::string name,
                     std::string default_value,
                     std::string description) {
  flag_data[name].default_value=default_value;
  flag_data[name]
    .registerer.reset(new google::FlagRegisterer(name.c_str(),
                                                 "string",
                                                 description.c_str(),
                                                 "python",
                                                 &flag_data[name].value,
                                              &flag_data[name].default_value));
}

std::string get_string_flag(std::string name) {
  std::string ret;
  google::GetCommandLineOption(name.c_str(), &ret);
  return ret;
}
#else
void add_string_flag(std::string,
                     std::string,
                     std::string) {
  IMP_LOG(WARNING, "Command line arguments requires gflags.");
}

std::string get_string_flag(std::string) {
  IMP_LOG(WARNING, "Command line arguments requires gflags.");
  return std::string();
}
#endif

IMPBASE_END_NAMESPACE

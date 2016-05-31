/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "base_static.h"
#if !IMP_KERNEL_HAS_LOG4CXX
#include "IMP/internal/log_stream.h"
#endif
#include "IMP/Pointer.h"
#include "IMP/Object.h"
#include "IMP/Showable.h"
#include "IMP/Vector.h"
#include "IMP/base_macros.h"
#include "IMP/flags.h"
#include "IMP/live_objects.h"
#include "boost/unordered_map.hpp"
#include "IMP/base_statistics.h"
#include "IMP/types.h"
#include <boost/unordered_set.hpp>
#include <IMP/random.h>
#include <boost/cstdint.hpp>
#include <boost/timer.hpp>
#include <cmath>
#include <ostream>
#if IMP_KERNEL_HAS_BOOST_RANDOM
#include <boost/nondet_random.hpp>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>   // open
#include <unistd.h>  // read, close
#endif

#if IMP_KERNEL_HAS_LOG4CXX
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/level.h>
#endif
IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
/*
 With all the static data in a single file, we
 needn't
 fuss about initialization and destruction
 order.
*/

// exceptions

// logging
bool print_time;
boost::timer log_timer;
#if !IMP_KERNEL_HAS_LOG4CXX
Vector<std::pair<const char*, const void*> > log_contexts;
int log_context_initializeds = -1;
unsigned int log_indent = 0;
std::ofstream fstream;
internal::LogStream stream;
#endif

/*
  With all the static data in a single file, we needn't
  fuss about initialization and destruction order.
 */

// objects

boost::unordered_map<std::string, unsigned int> object_type_counts;

#if IMP_KERNEL_HAS_LOG4CXX

namespace {
// cleaning up log4cxx doesn't seem to work very well
// with python
log4cxx::PatternLayout* layout = nullptr;
log4cxx::ConsoleAppender* appender = nullptr;
log4cxx::NDC* ndc = nullptr;
}

void init_logger() {
  if (!layout) {
    layout = new log4cxx::PatternLayout("%-4r %-5p [%x] - %m");
    appender = new log4cxx::ConsoleAppender(layout);
    log4cxx::BasicConfigurator::configure(appender);
    ndc = new log4cxx::NDC("IMP");
  }
}

#endif

boost::scoped_ptr<boost::progress_display> progress;

IMPKERNEL_END_INTERNAL_NAMESPACE
IMPKERNEL_BEGIN_NAMESPACE
unsigned int Object::live_objects_ = 0;
boost::unordered_set<Object*> live_;
bool show_live = true;
namespace {
struct CheckObjects {
  ~CheckObjects() {
    if (!live_.empty() && show_live) {
      std::cerr << "Not all IMP::Objects were freed prior to IMP"
                << " unloading. This is probably a bad thing." << std::endl;
      Strings names;
      int pushed = 0;
      for (boost::unordered_set<Object*>::const_iterator it = live_.begin();
           it != live_.end(); ++it) {
        names.push_back((*it)->get_name());
        ++pushed;
        if (pushed > 50) {
          names.push_back("...");
        }
      }
      std::cerr << names << std::endl;
    }
  }
};
CheckObjects check;
}

#if IMP_HAS_CHECKS
Strings get_live_object_names() {
  IMP::Vector<std::string> ret;
  for (boost::unordered_set<Object*>::const_iterator it = live_.begin();
       it != live_.end(); ++it) {
    ret.push_back((*it)->get_name());
  }
  return ret;
}
Objects get_live_objects() {
  Objects ret(live_.begin(), live_.end());
  return ret;
}
void set_show_leaked_objects(bool tf) { show_live = tf; }
#endif

#if IMP_HAS_CHECKS >= IMP_INTERNAL
void Object::add_live_object(Object* o) { live_.insert(o); }
void Object::remove_live_object(Object* o) {
  IMP_INTERNAL_CHECK(live_.find(o) != live_.end(),
                     "Object " << o->get_name() << " not found in live list.");
  live_.erase(o);
}
#endif
IMPKERNEL_END_NAMESPACE
IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
void check_live_objects() {
  for (boost::unordered_set<Object*>::const_iterator it = live_.begin();
       it != live_.end(); ++it) {
    IMP_USAGE_CHECK((*it)->get_ref_count() > 0,
                    "Object " << (*it)->get_name()
                    << " is alive but not ref counted - memory leakage possible."
                    << " This usually happens if an owning pointer is released"
                    << " without being either deleted manually or assigned to"
                    << " another owning pointer.");
  }
}

std::string exe_name, exe_usage, exe_description;

boost::program_options::options_description flags;
boost::program_options::options_description advanced_flags;
boost::program_options::variables_map variables_map;

Flag<bool> help("help,h", "Show command line arguments and exit.", false);
Flag<bool> version("version", "Show version info and exit.", false);
Flag<bool> help_advanced(
    "help_advanced",
    "Show all command line arguments including advanced ones and exit.", false);
AdvancedFlag<bool> show_seed("show_seed", "Print the random seed at startup.",
                             false);

AdvancedFlag<CheckLevel, IMP_HAS_CHECKS != IMP_NONE> check_level(
    "check_level",
    "The level of checking to use: \"NONE\", \"USAGE\" or "
    "\"USAGE_AND_INTERNAL\"",
    CheckLevel(IMP_HAS_CHECKS));

Flag<LogLevel, IMP_HAS_LOG != IMP_SILENT> log_level("log_level",
                                                    "The log level: "
                                                    "\"SILENT\", \"WARNING\", "
                                                    "\"PROGRESS\", \"TERSE\", "
                                                    "\"VERBOSE\"",
                                                    LogLevel(WARNING));

AdvancedFlag<StatisticsLevel> stats_level(
    "statistics_level",
    "The level of statistics to gather: \"NONE\" or \"ALL\".",
    StatisticsLevel(ALL_STATISTICS));

AdvancedFlag<bool, IMP_KERNEL_HAS_GPERFTOOLS> cpu_profile(
    "cpu_profile", "Perform CPU profiling.", false);
AdvancedFlag<bool, IMP_KERNEL_HAS_GPERFTOOLS> heap_profile(
    "heap_profile", "Perform heap profiling.", false);

AdvancedFlag<boost::int64_t, IMP_KERNEL_HAS_OPENMP> number_of_threads(
    "number_of_threads", "Number of threads to use.", 1);

namespace {
boost::uint64_t get_random_seed() {
#if IMP_KERNEL_HAS_BOOST_RANDOM
  return boost::random_device()();
#else
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd != -1) {
    boost::uint64_t result;
    int sz = read(fd, reinterpret_cast<char*>(&result), sizeof(result));
    if (sz == sizeof(result)) {
      return result;
    }
  }
  return static_cast<boost::uint64_t>(std::time(nullptr));
#endif
}
}
AdvancedFlag<boost::int64_t> random_seed("random_seed", "Random seed to use.",
                                         get_random_seed());

IMPKERNEL_END_INTERNAL_NAMESPACE

IMPKERNEL_BEGIN_NAMESPACE
::boost::mt19937 random_number_generator(
    static_cast<boost::int64_t>(internal::random_seed));

AdvancedFlag<bool> run_quick_test(
    "run_quick_test",
    "Run (quicker) tests on the program."
    " Not all executables do useful things with this.",
    false);
IMPKERNEL_END_NAMESPACE

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
AdvancedFlag<bool> no_print_deprecation_messages(
    "no_deprecation_warnings",
    "Don't print warnings on runtime deprecation use.", false);
AdvancedFlag<bool> exceptions_on_deprecation(
    "deprecation_exceptions",
    "Throw an exception when deprecated functions are used.", false);
boost::unordered_set<std::string> printed_deprecation_messages;

boost::unordered_map<std::string, Timing> timings;

namespace {
std::string timings_name;
AdvancedFlag<std::string> statistics(
    "statistics",
    "Writing statistics about various aspects to a file (or stdout)", "");

struct TimingsWriter {
  ~TimingsWriter() {
    if (timings_name == "stdout") {
      show_timings(std::cout);
    } else if (!timings_name.empty()) {
      show_timings(TextOutput(timings_name));
    }
  }
} tw;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

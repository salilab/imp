/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/internal/static.h"
#if !IMP_BASE_HAS_LOG4CXX
#include "IMP/base/internal/log_stream.h"
#endif
#include "IMP/base//Pointer.h"
#include "IMP/base/Object.h"
#include "IMP/base/Showable.h"
#include "IMP/base/Vector.h"
#include "IMP/base/base_macros.h"
#include "IMP/base/flags.h"
#include "IMP/base/live_objects.h"
#include "IMP/base/map.h"
#include "IMP/base/statistics.h"
#include "IMP/base/types.h"
#include <IMP/base/set.h>
#include <boost/cstdint.hpp>
#include <boost/timer.hpp>
#include <cmath>
#include <ostream>
#if IMP_BASE_HAS_BOOST_RANDOM
#include <boost/nondet_random.hpp>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>   // open
#include <unistd.h>  // read, close
#endif

#if IMP_BASE_HAS_LOG4CXX
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/level.h>
#endif
IMPBASE_BEGIN_INTERNAL_NAMESPACE
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
#if !IMP_BASE_HAS_LOG4CXX
base::Vector<std::pair<const char*, const void*> > log_contexts;
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

base::map<std::string, unsigned int> object_type_counts;

#if IMP_BASE_HAS_LOG4CXX

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

IMPBASE_END_INTERNAL_NAMESPACE
IMPBASE_BEGIN_NAMESPACE
unsigned int Object::live_objects_ = 0;
base::set<Object*> live_;
bool show_live = true;
namespace {
struct CheckObjects {
  ~CheckObjects() {
    if (!live_.empty() && show_live) {
      std::cerr << "Not all IMP::base::Objects were freed prior to IMP"
                << " unloading. This is probably a bad thing." << std::endl;
      Strings names;
      int pushed = 0;
      for (base::set<Object*>::const_iterator it = live_.begin();
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
  IMP::base::Vector<std::string> ret;
  for (base::set<Object*>::const_iterator it = live_.begin(); it != live_.end();
       ++it) {
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
IMPBASE_END_NAMESPACE
IMPBASE_BEGIN_INTERNAL_NAMESPACE
void check_live_objects() {
  for (base::set<Object*>::const_iterator it = live_.begin(); it != live_.end();
       ++it) {
    IMP_USAGE_CHECK((*it)->get_ref_count() > 0,
                    "Object " << (*it)->get_name() << " is not ref counted.");
  }
}

std::string exe_name, exe_usage, exe_description;

boost::program_options::options_description flags;
boost::program_options::options_description advanced_flags;
boost::program_options::variables_map variables_map;

Flag<bool> help("help", "Show command line arguments and exit.", false);
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
                                                    LogLevel(IMP_HAS_LOG));

AdvancedFlag<StatisticsLevel> stats_level(
    "statistics_level",
    "The level of statistics to gather: \"NONE\" or \"ALL\".",
    StatisticsLevel(ALL_STATISTICS));

AdvancedFlag<bool, IMP_BASE_HAS_GPERFTOOLS> cpu_profile(
    "cpu_profile", "Perform CPU profiling.", false);
AdvancedFlag<bool, IMP_BASE_HAS_GPERFTOOLS> heap_profile(
    "heap_profile", "Perform heap profiling.", &heap_profile);

AdvancedFlag<boost::int64_t, IMP_BASE_HAS_OPENMP> number_of_threads(
    "number_of_threads", "Number of threads to use.", 1);

namespace {
boost::uint64_t get_random_seed() {
#if IMP_BASE_HAS_BOOST_RANDOM
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

IMPBASE_END_INTERNAL_NAMESPACE

IMPBASE_BEGIN_NAMESPACE
::boost::mt19937 random_number_generator(
    static_cast<boost::int64_t>(internal::random_seed));

AdvancedFlag<bool> run_quick_test(
    "run_quick_test",
    "Run (quicker) tests on the program."
    " Not all executables do useful things with this.",
    false);
IMPBASE_END_NAMESPACE

IMPBASE_BEGIN_INTERNAL_NAMESPACE
AdvancedFlag<bool> no_print_deprecation_messages(
    "no_deprecation_warnings",
    "Don't print warnings on runtime deprecation use", false);
AdvancedFlag<bool> exceptions_on_deprecation(
    "deprecation_warnings", "Print warnings on runtime deprecation use", false);
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

IMPBASE_END_INTERNAL_NAMESPACE

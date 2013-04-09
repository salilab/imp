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
#include "IMP/base/RefCounted.h"
#include "IMP/base/Object.h"
#include <IMP/base/set.h>
#include "IMP/base/Showable.h"
#include "IMP/base//Pointer.h"
#include "IMP/base/RefCounted.h"
#include "IMP/base/base_macros.h"
#include "IMP/base/types.h"
#include "IMP/base/live_objects.h"
#include "IMP/base/map.h"
#include "IMP/base/Vector.h"
#include "IMP/base/flags.h"
#include <cmath>
#include <boost/timer.hpp>
#include <boost/cstdint.hpp>
#include <ostream>
#if IMP_BASE_HAS_BOOST_RANDOM
#include <boost/nondet_random.hpp>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    // open
#include <unistd.h>   // read, close
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
  With all the static data in a single file, we needn't
  fuss about initialization and destruction order.
 */


// exceptions

// logging
bool print_time;
boost::timer log_timer;
#if !IMP_BASE_HAS_LOG4CXX
unsigned int log_indent=0;
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
  log4cxx::PatternLayout* layout=nullptr;
  log4cxx::ConsoleAppender* appender=nullptr;
  log4cxx::NDC* ndc=nullptr;
}

void init_logger() {
  if (!layout) {
    layout = new log4cxx::PatternLayout("%-4r %-5p [%x] - %m");
    appender = new log4cxx::ConsoleAppender(layout);
    log4cxx::BasicConfigurator::configure(appender);
    ndc = new log4cxx::NDC("IMP");
    IMP_LOG_VERBOSE( "Initialized logging");
  }
}

#endif

IMPBASE_END_INTERNAL_NAMESPACE


IMPBASE_BEGIN_NAMESPACE
unsigned int RefCounted::live_objects_=0;
base::set<Object*> live_;
bool show_live=true;
namespace {
struct CheckObjects {
  ~CheckObjects() {
    if (!live_.empty() && show_live) {
      std::cerr << "Not all IMP::base::Objects were freed prior to IMP"
                << " unloading. This is probably a bad thing." << std::endl;
      Strings names;
      int pushed=0;
      for (base::set<Object*>::const_iterator
               it =live_.begin(); it != live_.end(); ++it) {
        names.push_back( (*it)->get_name());
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
Strings
get_live_object_names() {
  IMP::base::Vector<std::string> ret;
  for (base::set<Object*>::const_iterator it = live_.begin();
       it != live_.end(); ++it) {
    ret.push_back((*it)->get_name());
  }
  return ret;
}
Objects get_live_objects() {
  Objects ret(live_.begin(), live_.end());
  return ret;
}
void set_show_leaked_objects(bool tf) {
  show_live=tf;
}
#endif

#if IMP_HAS_CHECKS >= IMP_INTERNAL
void Object::add_live_object(Object*o) {
  live_.insert(o);
}
void Object::remove_live_object(Object*o) {
  IMP_INTERNAL_CHECK(live_.find(o) != live_.end(),
                     "Object " << o->get_name()
                     << " not found in live list.");
  live_.erase(o);
}
#endif
IMPBASE_END_NAMESPACE

IMPBASE_BEGIN_INTERNAL_NAMESPACE
void check_live_objects() {
  for (base::set<Object*>::const_iterator it= live_.begin();
       it != live_.end(); ++it) {
    IMP_USAGE_CHECK((*it)->get_ref_count()>0,
                    "Object " << (*it)->get_name() << " is not ref counted.");
  }
}
IMPBASE_END_INTERNAL_NAMESPACE

IMPBASE_BEGIN_INTERNAL_NAMESPACE

std::string exe_name, exe_usage, exe_description;

boost::program_options::options_description flags;
boost::program_options::variables_map variables_map;
namespace {
int default_check_level= IMP_HAS_CHECKS;
}

boost::int64_t check_level=default_check_level;

boost::int64_t log_level= TERSE;

#if IMP_HAS_CHECKS != IMP_NONE
AddIntFlag clf("check_level",
        "The level of checking to use: 0 for NONE, 1 for USAGE and 2 for ALL.",
               &check_level);
#endif
#if IMP_HAS_LOG != IMP_SILENT
AddIntFlag llf("log_level",
       "The log level, 0 for NONE, 1 for WARN, 2 for TERSE, 3 for VERBOSE",
               &log_level);
#endif

bool cpu_profile=false;
bool heap_profile=false;

#if IMP_BASE_HAS_GPERFTOOLS
AddBoolFlag cpf("cpu_profile", "Perform CPU profiling.", &cpu_profile);
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
AddBoolFlag hpf("heap_profile", "Perform heap profiling.", &heap_profile);
#endif

#ifdef _OPENMP
static const int default_number_of_threads=3;
#else
static const int default_number_of_threads=1;
#endif
int number_of_threads=default_number_of_threads;
#ifdef _OPENMP
AddIntFlag ntf("number_of_threads", "Number of threads to use.",
               &number_of_threads);
#endif

AddBoolFlag ntf("run_quick_test", "Run (quicker) tests on the program."\
                " Not all executables do useful things with this.",
                &run_quick_test);


namespace {
boost::uint64_t get_random_seed() {
#if IMP_BASE_HAS_BOOST_RANDOM
  //IMP_LOG_TERSE("Seeding from boost::random_device" << std::endl);
  return boost::random_device()();
#else
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd != -1) {
    //IMP_LOG_TERSE("Seeding from /dev/urandom" << std::endl);
    boost::uint64_t result;
    int sz = read(fd, reinterpret_cast<char *>(&result), sizeof(result));
    if(sz == sizeof(result)) {
      return result;
    }
  }
  //IMP_LOG_TERSE("Seeding from time" << std::endl);
  return  static_cast<boost::uint64_t>(std::time(nullptr));
#endif
}
}

boost::int64_t random_seed = get_random_seed();

AddIntFlag sf("random_seed", "Random seed to use.", &random_seed);

IMPBASE_END_INTERNAL_NAMESPACE

IMPBASE_BEGIN_NAMESPACE
::boost::rand48 random_number_generator(internal::random_seed);

bool run_quick_test = false;
IMPBASE_END_NAMESPACE

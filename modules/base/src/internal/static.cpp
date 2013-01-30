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
#include <IMP/compatibility/set.h>
#include "IMP/base/Showable.h"
#include "IMP/base//Pointer.h"
#include "IMP/base/RefCounted.h"
#include "IMP/base/base_macros.h"
#include "IMP/base/types.h"
#include "IMP/base/live_objects.h"
#include "IMP/compatibility/map.h"
#include "IMP/compatibility/vector.h"
#include "IMP/base/flags.h"
#include <cmath>
#include <boost/timer.hpp>
#include <ostream>
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

// The error message is already in the exception
bool print_exceptions=false;

// logging
bool print_time;
boost::timer log_timer;
#if !IMP_BASE_HAS_LOG4CXX
unsigned int log_indent=0;
IMP_CHECK_CODE(double initialized=11111111);
std::ofstream fstream;
internal::LogStream stream;
#endif

/*
  With all the static data in a single file, we needn't
  fuss about initialization and destruction order.
 */

// objects

compatibility::map<std::string, unsigned int> object_type_counts;


#if IMP_BASE_HAS_LOG4CXX
namespace {
struct Configurator {
  Configurator(log4cxx::ConsoleAppenderPtr ptr) {
    log4cxx::BasicConfigurator::configure(ptr);
  }
};
}
void init_logger() {
   // "%-4r [%t] %-5p %c %x - %m%n"
  static log4cxx::PatternLayoutPtr layout
      = new log4cxx::PatternLayout("%-4r %-5p [%x] - %m");
  static log4cxx::ConsoleAppenderPtr appender
      = new log4cxx::ConsoleAppender(layout);
  static Configurator config(appender);
  static log4cxx::NDC ndc("IMP");
  IMP_UNUSED(config);
}
#endif

IMPBASE_END_INTERNAL_NAMESPACE


IMPBASE_BEGIN_NAMESPACE
#if IMP_BUILD < IMP_FAST
unsigned int RefCounted::live_objects_=0;
compatibility::set<Object*> live_;
bool show_live=true;
namespace {
struct CheckObjects {
  ~CheckObjects() {
    if (!live_.empty() && show_live) {
      std::cerr << "Not all IMP::base::Objects were freed prior to IMP"
                << " unloading. This is probably a bad thing." << std::endl;
      Strings names;
      int pushed=0;
      for (compatibility::set<Object*>::const_iterator
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

Strings
get_live_object_names() {
  IMP::base::Vector<std::string> ret;
  for (compatibility::set<Object*>::const_iterator it = live_.begin();
       it != live_.end(); ++it) {
    ret.push_back((*it)->get_name());
  }
  return ret;
}
Objects get_live_objects() {
  Objects ret(live_.begin(), live_.end());
  return ret;
}
void Object::add_live_object(Object*o) {
  live_.insert(o);
}
void Object::remove_live_object(Object*o) {
  IMP_INTERNAL_CHECK(live_.find(o) != live_.end(),
                     "Object " << o->get_name()
                     << " not found in live list.");
  live_.erase(o);
}
void set_show_leaked_objects(bool tf) {
  show_live=tf;
}
#endif
IMPBASE_END_NAMESPACE

#if IMP_BUILD < IMP_FAST

IMPBASE_BEGIN_INTERNAL_NAMESPACE
void check_live_objects() {
  for (compatibility::set<Object*>::const_iterator it= live_.begin();
       it != live_.end(); ++it) {
    IMP_USAGE_CHECK((*it)->get_ref_count()>0,
                    "Object " << (*it)->get_name() << " is not ref counted.");
  }
}
IMPBASE_END_INTERNAL_NAMESPACE
#endif

IMPBASE_BEGIN_INTERNAL_NAMESPACE

std::string exe_name;

boost::program_options::options_description flags;
boost::program_options::variables_map variables_map;
namespace {
#if IMP_BUILD == IMP_FAST
int default_check_level= NONE;
#elif IMP_BUILD == IMP_RELEASE
  int default_check_level=check_level=USAGE;
#else
int default_check_level=USAGE_AND_INTERNAL;
#endif
}

int check_level=default_check_level;


int log_level= TERSE;
#if IMP_BUILD < IMP_FAST
AddIntFlag clf("check_level",
        "The level of checking to use: 0 for NONE, 1 for USAGE and 2 for ALL.",
               &check_level);

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

IMPBASE_END_INTERNAL_NAMESPACE

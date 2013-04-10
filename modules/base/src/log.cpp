/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/log.h"
#include "IMP/base/exception.h"
#include "IMP/base/file.h"
#include "IMP/base/internal/static.h"
#include "IMP/base/Object.h"
#include "IMP/base/thread_macros.h"
#ifdef _OPENMP
#include <omp.h>
#endif

#if IMP_BASE_HAS_LOG4CXX
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/level.h>
#else
#include "IMP/base/internal/log_stream.h"
#endif

IMPBASE_BEGIN_NAMESPACE
#if !IMP_BASE_HAS_LOG4CXX

namespace {
  base::Vector<std::pair<const char*, const void*> > contexts;
  int context_initializeds=-1;

  std::string get_context_name(unsigned int i) {
    std::ostringstream oss;
    if (contexts[i].second) {
      const Object *o= reinterpret_cast<const Object*>(contexts[i].second);
      // this is called on errors, so don't cause any more
      if (!o || !o->get_is_valid()) {
        oss << "InvalidObject" << "::" << contexts[i].first;
      } else {
        oss << o->get_name() << "::" << contexts[i].first;
      }
    } else {
      oss << contexts[i].first;
    }
    return oss.str();
  }

}


std::string get_context_message() {
  if (contexts.empty()) return std::string();
  std::ostringstream oss;
  oss << "\nContext: ";
  for (unsigned int i=0; i< contexts.size(); ++i) {
    if (i != 0) {
      oss << "/";
    }
    oss << get_context_name(i);
  }
  return oss.str();
}
#endif // Log4CXX

void set_log_level(LogLevel l) {
  // snap to max level
  if (l > IMP_HAS_LOG) {
    l = LogLevel(IMP_HAS_LOG);
  }
#if IMP_BASE_HAS_LOG4CXX
 try {
   switch (l) {
   case PROGRESS:
   case SILENT:
     get_logger()->setLevel(log4cxx::Level::getOff());
     break;
   case WARNING:
     get_logger()->setLevel(log4cxx::Level::getWarn());
     break;
   case TERSE:
     get_logger()->setLevel(log4cxx::Level::getInfo());
     break;
   case VERBOSE:
     get_logger()->setLevel(log4cxx::Level::getDebug());
     break;
   case MEMORY:
     get_logger()->setLevel(log4cxx::Level::getTrace());
     break;
   case DEFAULT:
   case ALL_LOG:
   default:
     IMP_WARN("Unknown log level " << boost::lexical_cast<std::string>(l));
   }
 } catch (log4cxx::helpers::Exception &) {
   IMP_THROW("Invalid log level", ValueException);
 }
#else
  IMP_USAGE_CHECK(l >= SILENT && l < ALL_LOG,
                  "Setting log to invalid level: " << l);
#endif
  IMP_OMP_PRAGMA(critical(imp_log))
  if (internal::log_level!=l ){
    internal::log_level=l;
  }
}

void set_log_target(TextOutput l)
{
#if IMP_BASE_HAS_LOG4CXX
  IMP_UNUSED(l);
#else
  internal::stream.set_stream(l);
#endif
}

TextOutput get_log_target()
{
#if IMP_BASE_HAS_LOG4CXX
  return TextOutput(std::cout);
#else
  return internal::stream.get_stream();
#endif
}


void set_log_timer(bool tb) {
#if IMP_BASE_HAS_LOG4CXX
  // always on for now
  IMP_UNUSED(tb);
#else
  internal::print_time=tb;
  reset_log_timer();
#endif
}

void reset_log_timer() {
#if IMP_BASE_HAS_LOG4CXX

#else
  internal::log_timer= boost::timer();
#endif
}

#if !IMP_BASE_HAS_LOG4CXX
void push_log_context(const char * functionname,
                                    const void * classname) {
  // we don't have multithread support
#ifdef _OPENMP
  if (!omp_in_parallel())
#endif
    {
      contexts.push_back(std::make_pair(functionname, classname));
    }
}

void pop_log_context() {
#ifdef _OPENMP
  if (!omp_in_parallel())
#endif
{
    if (context_initializeds >= static_cast<int>(contexts.size()-1)) {
      internal::log_indent-=2;
      std::string message= std::string("end ")
          +get_context_name(contexts.size()-1)+"\n";
      internal::stream.write(message.c_str(), message.size());
      internal::stream.strict_sync();
      -- context_initializeds;
    }
    contexts.pop_back();
  }
}
#endif

void add_to_log(LogLevel ll, std::string str) {
  IMP_LOG(ll, str);
  IMP_LOG_VARIABLE(ll);
  IMP_LOG_VARIABLE(str);
}

#if !IMP_BASE_HAS_LOG4CXX
void add_to_log(std::string str) {
  #pragma omp critical(imp_log)
  {
    if (!contexts.empty()
      && context_initializeds != static_cast<int>(contexts.size())) {
      for (unsigned int i=0; i< contexts.size(); ++i) {
        if (context_initializeds < static_cast<int>(i)) {
          std::string message= std::string("begin ")
          +get_context_name(i)+":\n";
          internal::stream.write(message.c_str(), message.size());
          internal::stream.strict_sync();
          internal::log_indent+=2;
          context_initializeds=i;
        }
      }
    }
    internal::stream.write(str.c_str(), str.size());
    internal::stream.strict_sync();
  }
}
#endif

void set_progress_display(std::string description,
                      unsigned int steps) {
  if (get_log_level() == PROGRESS) {
    IMP_USAGE_CHECK(!internal::progress, "There is already a progress bar.");
    std::cout << description << std::endl;
    internal::progress.reset(new boost::progress_display(steps));
  }
}

void add_to_progress_display(unsigned int step) {
  if (get_log_level() == PROGRESS) {
    IMP_USAGE_CHECK(internal::progress, "There is no progress bar.");
    for (unsigned int i = 0; i< step; ++i) {
      ++(*internal::progress);
    }
  }
}

IMPBASE_END_NAMESPACE

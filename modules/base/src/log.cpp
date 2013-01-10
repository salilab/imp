/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/log.h"
#include "IMP/base/internal/log_stream.h"
#include "IMP/base/exception.h"
#include "IMP/base/file.h"
#include "IMP/base/internal/static.h"
#include "IMP/base/Object.h"
#ifdef _OPENMP
#include <omp.h>
#endif

IMPBASE_BEGIN_NAMESPACE

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

void set_log_level(LogLevel l) {
  IMP_USAGE_CHECK(l >= SILENT && l < ALL_LOG,
            "Setting log to invalid level: " << l);
#pragma omp critical(imp_log)
  if (internal::log_level!=l ){
    internal::log_level=l;
    // creates too many useless messages, should be part of context
    //IMP_LOG(l, "Setting log level to " << l << std::endl);
  }
}

void set_log_target(TextOutput l)
{
  internal::stream.set_stream(l);
}

TextOutput get_log_target()
{
  return internal::stream.get_stream();
}

IMPBASEEXPORT void push_log_context(const char * functionname,
                                    const void * classname) {
  // we don't have multithread support
#ifdef _OPENMP
  if (!omp_in_parallel())
#endif
    {
      contexts.push_back(std::make_pair(functionname, classname));
    }
}

void set_log_timer(bool tb) {
  internal::print_time=tb;
  reset_log_timer();
}

void reset_log_timer() {
  internal::log_timer= boost::timer();
}


IMPBASEEXPORT void pop_log_context() {
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


void add_to_log(std::string str) {
  IMP_INTERNAL_CHECK(static_cast<int>(internal::initialized)==11111111,
                     "You connot use the log before main is called.");
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

IMPBASE_END_NAMESPACE

/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/exception.h"
#include "IMP/file.h"
#include "IMP/Object.h"
#include "internal/static.h"
#include <IMP/internal/SimpleTimer.h>
#include "IMP/thread_macros.h"
#ifdef _OPENMP
#include <omp.h>
#endif

#if IMP_KERNEL_HAS_LOG4CXX
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/level.h>
#else
#include "IMP/internal/log_stream.h"
#endif

IMPKERNEL_BEGIN_NAMESPACE
#if !IMP_KERNEL_HAS_LOG4CXX
namespace {
std::string get_context_name(unsigned int i) {
  std::ostringstream oss;
  if (internal::log_contexts[i].second) {
    if (!internal::log_contexts[i].second) {
      oss << internal::log_contexts[i].first;
    } else {
      const Object *o =
          reinterpret_cast<const Object *>(internal::log_contexts[i].second);
      // this is called on errors, so don't cause any more
      if (!o->get_is_valid()) {
        oss << "InvalidObject"
            << "::" << internal::log_contexts[i].first;
      } else {
        oss << o->get_name() << "::" << internal::log_contexts[i].first;
      }
    }
  } else {
    oss << internal::log_contexts[i].first;
  }
  return oss.str();
}
}

std::string get_context_message() {
  if (internal::log_contexts.empty()) return std::string();
  std::ostringstream oss;
  oss << "\nContext: ";
  for (unsigned int i = 0; i < internal::log_contexts.size(); ++i) {
    if (i != 0) {
      oss << "/";
    }
    oss << get_context_name(i);
  }
  return oss.str();
}
#endif  // Log4CXX

void set_log_level(LogLevel l) {
  // snap to max level
  if (l > IMP_HAS_LOG) {
    l = LogLevel(IMP_HAS_LOG);
  }
#if IMP_KERNEL_HAS_LOG4CXX
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
  }
  catch (log4cxx::helpers::Exception &) {
    IMP_THROW("Invalid log level", ValueException);
  }
#else
  IMP_USAGE_CHECK(l >= SILENT && l < ALL_LOG,
                  "Setting log to invalid level: " << l);
#endif
  IMP_OMP_PRAGMA(critical(imp_log))
  if (internal::log_level != l) {
    internal::log_level = l;
  }
}

void set_log_target(TextOutput l) {
#if IMP_KERNEL_HAS_LOG4CXX
  IMP_UNUSED(l);
#else
  internal::stream.set_stream(l);
#endif
}

TextOutput get_log_target() {
#if IMP_KERNEL_HAS_LOG4CXX
  return TextOutput(std::cout);
#else
  return internal::stream.get_stream();
#endif
}

void set_log_timer(bool tb) {
#if IMP_KERNEL_HAS_LOG4CXX
  // always on for now
  IMP_UNUSED(tb);
#else
  internal::print_time = tb;
  reset_log_timer();
#endif
}

void reset_log_timer() {
#if IMP_KERNEL_HAS_LOG4CXX

#else
  internal::log_timer = internal::SimpleTimer();
#endif
}

#if !IMP_KERNEL_HAS_LOG4CXX
void push_log_context(const char *functionname, const void *classname) {
// we don't have multithread support
#ifdef _OPENMP
  if (!omp_in_parallel())
#endif
  {
    internal::log_contexts.push_back(std::make_pair(functionname, classname));
  }
}

void pop_log_context() {
#ifdef _OPENMP
  if (!omp_in_parallel())
#endif
  {
    if (internal::log_context_initializeds >=
        static_cast<int>(internal::log_contexts.size() - 1)) {
      internal::log_indent -= 2;
      std::string message =
          std::string("end ") +
          get_context_name(internal::log_contexts.size() - 1) + "\n";
      internal::stream.write(message.c_str(), message.size());
      internal::stream.strict_sync();
      --internal::log_context_initializeds;
    }
    internal::log_contexts.pop_back();
  }
}
#endif

void add_to_log(LogLevel ll, std::string str) {
  IMP_LOG(ll, str);
  IMP_LOG_VARIABLE(ll);
  IMP_LOG_VARIABLE(str);
}

#if !IMP_KERNEL_HAS_LOG4CXX
void add_to_log(std::string str) {
#ifdef _OPENMP
#pragma omp critical(imp_log)
#endif
  {
    if (!internal::log_contexts.empty() &&
        internal::log_context_initializeds !=
            static_cast<int>(internal::log_contexts.size())) {
      for (unsigned int i = 0; i < internal::log_contexts.size(); ++i) {
        if (internal::log_context_initializeds < static_cast<int>(i)) {
          std::string message =
              std::string("begin ") + get_context_name(i) + ":\n";
          internal::stream.write(message.c_str(), message.size());
          internal::stream.strict_sync();
          internal::log_indent += 2;
          internal::log_context_initializeds = i;
        }
      }
    }
    internal::stream.write(str.c_str(), str.size());
    internal::stream.strict_sync();
  }
}
#endif

void set_progress_display(std::string description, unsigned int steps) {
  if (get_log_level() == PROGRESS) {
    IMP_USAGE_CHECK(!internal::progress, "There is already a progress bar.");
    std::cout << description << std::endl;
    internal::progress.reset(new IMP::internal::BoostProgressDisplay(steps));
  }
}

void add_to_progress_display(unsigned int step) {
  if (get_log_level() == PROGRESS) {
    IMP_USAGE_CHECK(internal::progress, "There is no progress bar.");
    for (unsigned int i = 0; i < step; ++i) {
      ++(*internal::progress);
    }
    if (internal::progress->count() == internal::progress->expected_count()) {
      internal::progress.reset();
    }
  }
}

IMPKERNEL_END_NAMESPACE

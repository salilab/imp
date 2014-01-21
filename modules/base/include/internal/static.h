/**
 *  \file internal/static.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_STATIC_H
#define IMPBASE_INTERNAL_STATIC_H

#include <IMP/base/base_config.h>
#include <IMP/base/Vector.h>
#include <IMP/base/map.h>
#include <IMP/base/Flag.h>
#include <IMP/base/tuple_macros.h>
#include <IMP/base/enums.h>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_set.hpp>
#include <boost/progress.hpp>
#include <boost/scoped_ptr.hpp>

IMPBASE_BEGIN_INTERNAL_NAMESPACE
extern IMPBASEEXPORT bool print_time;

extern IMPBASEEXPORT base::map<std::string, unsigned int> object_type_counts;

IMPBASEEXPORT void check_live_objects();

extern IMPBASEEXPORT boost::program_options::options_description flags;
extern IMPBASEEXPORT boost::program_options::options_description advanced_flags;
extern IMPBASEEXPORT boost::program_options::variables_map variables_map;

extern IMPBASEEXPORT AdvancedFlag<CheckLevel, IMP_HAS_CHECKS != IMP_NONE>
    check_level;
extern IMPBASEEXPORT AdvancedFlag<StatisticsLevel> stats_level;
#if !IMP_BASE_HAS_LOG4CXX
extern IMPBASEEXPORT base::Vector<std::pair<const char *, const void *> >
    log_contexts;
extern IMPBASEEXPORT int log_context_initializeds;
extern IMPBASEEXPORT unsigned int log_indent;
class LogStream;
extern IMPBASEEXPORT LogStream stream;
#endif
// needed for flags
extern IMPBASEEXPORT Flag<LogLevel, IMP_HAS_LOG != IMP_SILENT> log_level;

// Should be uint, but no flags support for that.
extern IMPBASEEXPORT AdvancedFlag<boost::int64_t> random_seed;

extern IMPBASEEXPORT AdvancedFlag<bool, IMP_BASE_HAS_GPERFTOOLS> cpu_profile;
extern IMPBASEEXPORT AdvancedFlag<bool, IMP_BASE_HAS_GPERFTOOLS> heap_profile;
extern IMPBASEEXPORT Flag<bool> help;
extern IMPBASEEXPORT Flag<bool> help_advanced;
extern IMPBASEEXPORT Flag<bool> version;
extern IMPBASEEXPORT AdvancedFlag<bool> show_seed;

extern IMPBASEEXPORT std::string exe_name;
extern IMPBASEEXPORT std::string exe_usage;
extern IMPBASEEXPORT std::string exe_description;

#ifdef _OPENMP
#define IMP_BASE_HAS_OPENMP 1
#else
#define IMP_BASE_HAS_OPENMP 0
#endif

extern IMPBASEEXPORT AdvancedFlag<boost::int64_t, IMP_BASE_HAS_OPENMP>
    number_of_threads;

extern IMPBASEEXPORT boost::scoped_ptr<boost::progress_display> progress;

extern IMPBASEEXPORT AdvancedFlag<bool> no_print_deprecation_messages;
extern IMPBASEEXPORT AdvancedFlag<bool> exceptions_on_deprecation;
extern IMPBASEEXPORT boost::unordered_set<std::string>
    printed_deprecation_messages;

struct Timing {
  double total_time;
  unsigned int calls;
  Timing() : total_time(0), calls(0) {}
};
extern IMPBASEEXPORT boost::unordered_map<std::string, Timing> timings;
IMPBASE_END_INTERNAL_NAMESPACE

#endif /* IMPBASE_INTERNAL_STATIC_H */

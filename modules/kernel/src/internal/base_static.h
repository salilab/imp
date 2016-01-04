/**
 *  \file internal/static.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_BASE_STATIC_H
#define IMPKERNEL_BASE_STATIC_H

#include <IMP/kernel_config.h>
#include <IMP/internal/base_static.h>
#include <IMP/tuple_macros.h>
#include <IMP/enums.h>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/progress.hpp>
#include <boost/scoped_ptr.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
extern IMPKERNELEXPORT bool print_time;

extern IMPKERNELEXPORT boost::unordered_map<std::string, unsigned int>
    object_type_counts;

IMPKERNELEXPORT void check_live_objects();

extern IMPKERNELEXPORT boost::program_options::options_description flags;
extern IMPKERNELEXPORT boost::program_options::options_description advanced_flags;
extern IMPKERNELEXPORT boost::program_options::variables_map variables_map;

extern IMPKERNELEXPORT AdvancedFlag<CheckLevel, IMP_HAS_CHECKS != IMP_NONE>
    check_level;
extern IMPKERNELEXPORT AdvancedFlag<StatisticsLevel> stats_level;
#if !IMP_KERNEL_HAS_LOG4CXX
extern IMPKERNELEXPORT Vector<std::pair<const char *, const void *> >
    log_contexts;
extern IMPKERNELEXPORT int log_context_initializeds;
extern IMPKERNELEXPORT unsigned int log_indent;
class LogStream;
extern IMPKERNELEXPORT LogStream stream;
#endif
// needed for flags
extern IMPKERNELEXPORT Flag<LogLevel, IMP_HAS_LOG != IMP_SILENT> log_level;

// Should be uint, but no flags support for that.
extern IMPKERNELEXPORT AdvancedFlag<boost::int64_t> random_seed;

extern IMPKERNELEXPORT AdvancedFlag<bool, IMP_KERNEL_HAS_GPERFTOOLS> cpu_profile;
extern IMPKERNELEXPORT AdvancedFlag<bool, IMP_KERNEL_HAS_GPERFTOOLS> heap_profile;
extern IMPKERNELEXPORT Flag<bool> help;
extern IMPKERNELEXPORT Flag<bool> help_advanced;
extern IMPKERNELEXPORT Flag<bool> version;
extern IMPKERNELEXPORT AdvancedFlag<bool> show_seed;

extern IMPKERNELEXPORT std::string exe_name;
extern IMPKERNELEXPORT std::string exe_usage;
extern IMPKERNELEXPORT std::string exe_description;

extern IMPKERNELEXPORT AdvancedFlag<boost::int64_t, IMP_KERNEL_HAS_OPENMP>
    number_of_threads;

extern IMPKERNELEXPORT boost::scoped_ptr<boost::progress_display> progress;

extern IMPKERNELEXPORT AdvancedFlag<bool> no_print_deprecation_messages;
extern IMPKERNELEXPORT AdvancedFlag<bool> exceptions_on_deprecation;
extern IMPKERNELEXPORT boost::unordered_set<std::string>
    printed_deprecation_messages;

struct Timing {
  double total_time;
  unsigned int calls;
  Timing() : total_time(0), calls(0) {}
};
extern IMPKERNELEXPORT boost::unordered_map<std::string, Timing> timings;
IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_BASE_STATIC_H */

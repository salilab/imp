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
#include <IMP/base/Flag.h>
#include <IMP/base/enums.h>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

IMPBASE_BEGIN_INTERNAL_NAMESPACE
extern IMPBASEEXPORT bool print_time;

IMPBASEEXPORT void check_live_objects();

extern IMPBASEEXPORT boost::program_options::options_description flags;
extern IMPBASEEXPORT boost::program_options::options_description advanced_flags;
extern IMPBASEEXPORT boost::program_options::variables_map variables_map;

extern IMPBASEEXPORT AdvancedFlag<CheckLevel, IMP_HAS_CHECKS != IMP_NONE>
    check_level;
extern IMPBASEEXPORT AdvancedFlag<StatisticsLevel> stats_level;

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

#ifdef _OPENMP
#define IMP_BASE_HAS_OPENMP 1
#else
#define IMP_BASE_HAS_OPENMP 0
#endif

IMPBASE_END_INTERNAL_NAMESPACE

#endif /* IMPBASE_INTERNAL_STATIC_H */

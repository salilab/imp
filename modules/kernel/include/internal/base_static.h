/**
 *  \file internal/static.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_BASE_STATIC_H
#define IMPKERNEL_INTERNAL_BASE_STATIC_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <IMP/Flag.h>
#include <IMP/enums.h>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
extern IMPKERNELEXPORT bool print_time;

IMPKERNELEXPORT void check_live_objects();

extern IMPKERNELEXPORT boost::program_options::options_description flags;
extern IMPKERNELEXPORT boost::program_options::options_description advanced_flags;
extern IMPKERNELEXPORT boost::program_options::variables_map variables_map;

extern IMPKERNELEXPORT AdvancedFlag<CheckLevel, IMP_HAS_CHECKS != IMP_NONE>
    check_level;

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

#ifdef _OPENMP
#define IMP_KERNEL_HAS_OPENMP 1
#else
#define IMP_KERNEL_HAS_OPENMP 0
#endif

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_BASE_STATIC_H */

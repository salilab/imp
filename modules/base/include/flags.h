/**
 *  \file IMP/base/flags.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_FLAGS_H
#define IMPBASE_FLAGS_H

#include <IMP/base/base_config.h>
#include <IMP/base/Vector.h>
#include "flag_macros.h"
#include <string>

#if defined(IMP_BASE_USE_GFLAGS)
#include <gflags/gflags.h>
#endif

IMPBASE_BEGIN_NAMESPACE

#if defined(IMP_BASE_USE_GFLAGS)

inline void set_usage_message(std::string usage) {
  google::SetUsageMessage(usage);
}

inline void set_version_string(std::string version) {
  google::SetVersionString(version);
}

#else
inline void set_usage_message(std::string) {
}

inline void set_version_string(std::string) {
}
#endif

/** Return the name of the current executable.*/
IMPBASEEXPORT std::string get_executable_name();

#ifndef SWIG
/** Parse the command line flags and return the
    positional arguments.
    \note This modifies argv so it is not safe to reuse it.
    */
IMPBASEEXPORT base::Vector<std::string>
setup_from_argv(int argc, char **argv,
                int num_positional);
#endif

IMPBASEEXPORT base::Vector<std::string>
setup_from_argv(base::Vector<std::string> argv,
                int num_positional);

/** For python use.*/
IMPBASEEXPORT void add_string_flag(std::string name,
                                   std::string default_value,
                                   std::string description);
/** For python use.*/
IMPBASEEXPORT std::string get_string_flag(std::string name);

IMP_DECLARE_INT(BASE, check_level);
IMP_DECLARE_INT(BASE, log_level);

IMPBASE_END_NAMESPACE


#endif  /* IMPBASE_FLAGS_H */

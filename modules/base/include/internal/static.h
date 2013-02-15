/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_STATIC_H
#define IMPBASE_INTERNAL_STATIC_H

#include <IMP/base/base_config.h>
#include "../flags.h"
#include <IMP/base/map.h>
#include <boost/program_options.hpp>


IMPBASE_BEGIN_INTERNAL_NAMESPACE

extern IMPBASEEXPORT bool print_time;

extern IMPBASEEXPORT base::map<std::string,
                                    unsigned int> object_type_counts;

IMPBASEEXPORT void check_live_objects();

extern IMPBASEEXPORT boost::program_options::options_description flags;
extern IMPBASEEXPORT boost::program_options::variables_map variables_map;

extern IMPBASEEXPORT int check_level;
#if !IMP_BASE_HAS_LOG4CXX
extern IMPBASEEXPORT unsigned int log_indent;
class LogStream;
extern IMPBASEEXPORT LogStream stream;
#endif
// needed for flags
extern IMPBASEEXPORT int log_level;

extern IMPBASEEXPORT bool cpu_profile;
extern IMPBASEEXPORT bool heap_profile;

extern IMPBASEEXPORT std::string exe_name;

extern IMPBASEEXPORT int number_of_threads;

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_STATIC_H */

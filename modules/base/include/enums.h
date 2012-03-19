/**
 *  \file IMP/base/enums.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_ENUMS_H
#define IMPBASE_ENUMS_H

#include "base_config.h"
//IMPBASE_BEGIN_NAMESPACE
namespace IMP {

namespace base {
//! The log levels supported by \imp
/**
    DEFAULT is only local logging (like in IMP::Object); it means to use
    the global log level.

    VERBOSE prints very large amounts of information. It should be enough
    to allow the computational flow to be understood.

    TERSE prints a few lines per restraint or per state each time
    the score is evaluated.

    PROGRESS show how large calculations are progressing (eg samplers)
    but otherwise just show warnings

    WARNING prints only warnings.

    MEMORY prints information about allocations and deallocations to debug
    memory issues.
 */
enum LogLevel {DEFAULT=-1, SILENT=0, WARNING=1, PROGRESS=2,
               TERSE=3, VERBOSE=4,
               MEMORY=5
#ifndef IMP_DOXYGEN
               , ALL_LOG
#endif
};

// duplicated in IMP_base_exception.i otherwise IMP_base_exception.i
// is processed without seeing this definition yet.
#ifndef SWIG
//! Determine the level of runtime checks performed
/** - NONE means that minimial checks are
    used.
    - USAGE means that checks of input values to functions
    and classes are verified.
    - USAGE_AND_INTERNAL adds checks that \imp itself is
    correct. Turn these on if you suspect an \imp bug or are
    developing Restraints or other \imp classes.
*/
enum CheckLevel {DEFAULT_CHECK=-1, NONE=0, USAGE=1, USAGE_AND_INTERNAL=2};
#endif
}
} //IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_ENUMS_H */

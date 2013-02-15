/**
 *  \file IMP/base/enums.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_ENUMS_H
#define IMPBASE_ENUMS_H

#include <IMP/base/base_config.h>
#include "compiler_macros.h"
//IMPBASE_BEGIN_NAMESPACE
namespace IMP {

namespace base {
//! The log levels supported by \imp
enum LogLevel {
  /** Use to specify that the global log level should be used
      (eg  in IMP::base::Object::set_log_level())*/
  DEFAULT = -1,
  /** Do not output any text.*/
  SILENT = IMP_SILENT,
  /** Output only warnings.*/
  WARNING = IMP_SILENT+1,
  /** Output only progress meter style displays and occasional printouts
      when switching phases of work.*/
  PROGRESS = IMP_PROGRESS,
  /** Output a line or two per evaluation call.*/
  TERSE = IMP_TERSE,
  /** Produce copious output to allow someone to trace through
      the computation.
  */
  VERBOSE = IMP_VERBOSE,
  /** Log memory allocations and frees.*/
  MEMORY = IMP_VERBOSE+1
#ifndef IMP_DOXYGEN
  , ALL_LOG=6
#endif
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPBASEEXPORT std::istream &operator>>(std::istream &in,
                                       LogLevel &ll);
IMPBASEEXPORT std::ostream &operator<<(std::ostream &in,
                                       LogLevel &ll);
#endif

// duplicated in IMP_base_exception.i otherwise IMP_base_exception.i
// is processed without seeing this definition yet.
#ifndef SWIG
//! Specify the level of runtime checks performed
enum CheckLevel {
  /** Use the default check level (eg IMP::base::Object::set_check_level()).*/
  DEFAULT_CHECK = -1,
  /** Perform no runtime checks.*/
  NONE = IMP_NONE,
  /** Perform checks that \imp is being called correctly.*/
  USAGE = IMP_USAGE,
  /** Check internal \imp invariants. This is to be used for debugging
      \imp.*/
  USAGE_AND_INTERNAL = IMP_INTERNAL};
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPBASEEXPORT std::istream &operator>>(std::istream &in,
                                       CheckLevel &ll);
#endif
}
} //IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_ENUMS_H */

/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/deprecation.h"
#include "internal/base_static.h"
#include "IMP/exception.h"
#include "IMP/log_macros.h"

IMPKERNEL_BEGIN_NAMESPACE
void handle_use_deprecated(std::string message) {
  if (get_log_level() >= WARNING /* Make sure the message is shown once */
      && !internal::no_print_deprecation_messages) {
    if (internal::printed_deprecation_messages.find(message) ==
        internal::printed_deprecation_messages.end()) {
      IMP_WARN(message);
      internal::printed_deprecation_messages.insert(message);
    }
  }
// we only have a stacktrace if logging is on
#if IMP_HAS_LOG > IMP_SILENT
  if (internal::exceptions_on_deprecation) {
    throw UsageException(message.c_str());
  }
#endif
}

void set_deprecation_warnings(bool tf) {
  internal::no_print_deprecation_messages = !tf;
}

void set_deprecation_exceptions(bool tf) {
  internal::exceptions_on_deprecation = tf;
}

bool get_deprecation_exceptions() {
  return internal::exceptions_on_deprecation;
}

IMPKERNEL_END_NAMESPACE

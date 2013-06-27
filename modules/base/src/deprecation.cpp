/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/deprecation.h"
#include "IMP/base/internal/static.h"
#include "IMP/base/exception.h"
#include "IMP/base/log_macros.h"

IMPBASE_BEGIN_NAMESPACE
void handle_use_deprecated(std::string message) {
  if (internal::print_deprecation_messages) {
    if (internal::printed_deprecation_messages.find(message) ==
        internal::printed_deprecation_messages.end()) {
      IMP_WARN(message);
      internal::printed_deprecation_messages.insert(message);
    }
  }
  if (internal::exceptions_on_deprecation) {
    throw UsageException(message.c_str());
  }
}

void set_deprecation_warnings(bool tf) {
  internal::print_deprecation_messages = tf;
}

void set_deprecation_exceptions(bool tf) {
  internal::exceptions_on_deprecation = tf;
}

IMPBASE_END_NAMESPACE

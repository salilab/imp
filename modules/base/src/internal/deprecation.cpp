/**
 *  \file deprecation.cpp   \brief classes for deprecation.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/deprecation.h"
#include "IMP/base/log.h"
#include "IMP/base/log_macros.h"
#include "IMP/base/set.h"

IMPBASE_BEGIN_INTERNAL_NAMESPACE

namespace {

  bool print_deprecation_messages=true;

  base::set<std::string> printed_deprecation_messages;

}

bool get_print_deprecation_message(std::string name) {
  return print_deprecation_messages
    && printed_deprecation_messages.find(name)
    == printed_deprecation_messages.end();
}

void set_printed_deprecation_message(std::string name, bool tr) {
  if (tr) {
    printed_deprecation_messages.insert(name);
  } else {
    printed_deprecation_messages.erase(name);
  }
}

IMPBASE_END_INTERNAL_NAMESPACE


IMPBASE_BEGIN_NAMESPACE

void set_print_deprecation_messages(bool tf) {
  internal::print_deprecation_messages=tf;
  IMP_LOG_TERSE( "Printing of deprecation messages is now " << tf << std::endl);
}

IMPBASE_END_NAMESPACE

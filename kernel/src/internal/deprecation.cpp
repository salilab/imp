/**
 *  \file deprecation.cpp   \brief classes for deprecation.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/deprecation.h"
#include "IMP/log.h"

#include <set>

IMP_BEGIN_INTERNAL_NAMESPACE

namespace {

bool print_deprecation_messages=true;

std::set<std::string> printed_deprecation_messages;

}

bool get_print_deprecation_message(const char *name) {
  return print_deprecation_messages
    && printed_deprecation_messages.find(name)
    == printed_deprecation_messages.end();
}

void set_printed_deprecation_message(const char *name, bool tr) {
  if (tr) {
    printed_deprecation_messages.insert(name);
  } else {
    printed_deprecation_messages.erase(name);
  }
}

IMP_END_INTERNAL_NAMESPACE


IMP_BEGIN_NAMESPACE

void set_print_deprecation_messages(bool tf) {
  internal::print_deprecation_messages=tf;
  IMP_LOG(TERSE, "Printing of deprecation messages is now " << tf << std::endl);
}

IMP_END_NAMESPACE

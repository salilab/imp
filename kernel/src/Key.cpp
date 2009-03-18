/**
 *  \file Key.cpp   \brief Internal workings of keys.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Key.h"
#include "IMP/exception.h"

IMP_BEGIN_INTERNAL_NAMESPACE

static double heuristic_value=238471628;

KeyData::KeyData(): heuristic_(heuristic_value){}

void KeyData::assert_is_initialized() const
{
  IMP_assert(heuristic_== heuristic_value,
             "Uninitialized KeyData. Do not initialize Keys statically.");
}

void KeyData::show(std::ostream &out) const
{
  for (unsigned int i=0; i< rmap_.size(); ++i) {
    out << "\"" << rmap_[i] << "\" ";
  }
}

IMPEXPORT KeyData& get_key_data(unsigned int index) {
  static std::map<unsigned int, KeyData> key_data;
  if (key_data.find(index) == key_data.end()) {
    IMP_LOG(VERBOSE, "Initializing keys with index " << index << std::endl);
  }
  return key_data[index];
}

IMP_END_INTERNAL_NAMESPACE

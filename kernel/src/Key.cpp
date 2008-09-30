/**
 *  \file Key.cpp   \brief Internal workings of keys.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Key.h"
#include "IMP/exception.h"

IMP_BEGIN_NAMESPACE

namespace internal
{

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


std::map<unsigned int, KeyData> key_data;

} // namespace internal

IMP_END_NAMESPACE

/**
 *  \file Key.cpp   \brief Internal workings of keys.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Key.h"
#include "IMP/base//exception.h"
#include "IMP/kernel/base_types.h"
#include "IMP/base//map.h"
#include "IMP/kernel/Particle.h"
#include "IMP/kernel/internal/AttributeTable.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


// keys
static double heuristic_value=238471628;

namespace {
  struct KeyTable: public base::map<unsigned int, KeyData> {
    KeyTable() {
      unsigned int fk= FloatKey::get_ID();
      operator[](fk).add_key("x");
      operator[](fk).add_key("y");
      operator[](fk).add_key("z");
      operator[](fk).add_key("radius");
      operator[](fk).add_key("local_x");
      operator[](fk).add_key("local_y");
      operator[](fk).add_key("local_z");
    }
  };
}

KeyData& get_key_data(unsigned int index) {
  static KeyTable key_data;
  return key_data[index];
}


KeyData::KeyData(): heuristic_(heuristic_value){}

void KeyData::assert_is_initialized() const
{
#if IMP_HAS_CHECKS < 2
  IMP_UNUSED(heuristic_);
#endif
  IMP_INTERNAL_CHECK(static_cast<int>(heuristic_)
                     == static_cast<int>(heuristic_value),
             "Uninitialized KeyData. Do not initialize Keys statically.");
}

void KeyData::show(std::ostream &out) const
{
  for (unsigned int i=0; i< rmap_.size(); ++i) {
    out << "\"" << rmap_[i] << "\" ";
  }
}


IMPKERNEL_END_INTERNAL_NAMESPACE

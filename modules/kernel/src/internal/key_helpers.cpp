/**
 *  \file key_helpers.cpp   \brief Internal workings of keys.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/key_helpers.h"
#include "IMP/Key.h"
#include "IMP/exception.h"
#include "IMP/base_types.h"
#include "IMP/check_macros.h"
#include "IMP/Particle.h"
#include "IMP/internal/AttributeTable.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE


// keys
static double heuristic_value = 238471628;

KeyData::KeyData(
#ifndef IMPKERNEL_INTERNAL_OLD_COMPILER
		 unsigned int ID
#endif
) : heuristic_(heuristic_value)
{
#ifndef IMPKERNEL_INTERNAL_OLD_COMPILER
  //  IMP_LOG_PROGRESS("KeyData::KeyData ID " << ID << std::endl);
  // float keys are special cased for historical reasons
  if(ID==FloatKey::get_ID()){
    // IMP_LOG_PROGRESS("Adding special float keys" << std::endl);
    unsigned int x= add_key("x");
    unsigned int y= add_key("y");
    unsigned int z= add_key("z");
    unsigned int radius= add_key("radius");
    unsigned int x_local= add_key("x_local");
    unsigned int y_local= add_key("y_local");
    unsigned int z_local= add_key("z_local");
    IMP_UNUSED(x);
    IMP_UNUSED(y);
    IMP_UNUSED(z);
    IMP_UNUSED(radius);
    IMP_UNUSED(x_local);
    IMP_UNUSED(y_local);
    IMP_UNUSED(z_local);
    IMP_INTERNAL_CHECK(x==0, "x key should be 0");
    IMP_INTERNAL_CHECK(y==1, "y key should be 1");
    IMP_INTERNAL_CHECK(z==2, "z key should be 2");
    IMP_INTERNAL_CHECK(radius==3, "radius key should be 3");
    IMP_INTERNAL_CHECK(x_local==4, "x_local key should be 4");
    IMP_INTERNAL_CHECK(y_local==5, "y_local key should be 5");
    IMP_INTERNAL_CHECK(z_local==6, "z_local key should be 6");
  }
#endif
}

#ifdef IMPKERNEL_INTERNAL_OLD_COMPILER
namespace {
  struct KeyTable : public boost::unordered_map<unsigned int, KeyData> {
    KeyTable() {
      static unsigned int fk = FloatKey::get_ID();
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
  static  KeyTable key_data;
  return key_data[index];
}
#endif

void KeyData::assert_is_initialized() const {
#if IMP_HAS_CHECKS < 2
  IMP_UNUSED(heuristic_);
#endif
  IMP_INTERNAL_CHECK(
      static_cast<int>(heuristic_) == static_cast<int>(heuristic_value),
      "Uninitialized KeyData. Do not initialize Keys statically.");
}

void KeyData::show(std::ostream& out) const {
  for (unsigned int i = 0; i < rmap_.size(); ++i) {
    out << "\"" << rmap_[i] << "\" ";
  }
}

IMPKERNEL_END_INTERNAL_NAMESPACE

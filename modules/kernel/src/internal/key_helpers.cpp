/**
 *  \file key_helpers.cpp   \brief Internal workings of keys.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
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


KeyData::KeyData(unsigned int ID) : heuristic_(heuristic_value)
{
  // float keys are special cased for historical reasons
  if(ID==FloatKey::get_ID()){
    IMP_LOG_PROGRESS("Adding special float keys" << std::endl);
    unsigned int x= add_key("x");
    unsigned int y= add_key("y");
    unsigned int z= add_key("z");
    unsigned int radius= add_key("radius");
    unsigned int x_local= add_key("x_local");
    unsigned int y_local= add_key("y_local");
    unsigned int z_local= add_key("z_local");
    IMP_INTERNAL_CHECK(x==0, "x key should be 0");
    IMP_INTERNAL_CHECK(y==1, "y key should be 1");
    IMP_INTERNAL_CHECK(z==2, "z key should be 2");
    IMP_INTERNAL_CHECK(radius==3, "radius key should be 3");
    IMP_INTERNAL_CHECK(x_local==4, "x_local key should be 4");
    IMP_INTERNAL_CHECK(y_local==5, "y_local key should be 5");
    IMP_INTERNAL_CHECK(z_local==6, "z_local key should be 6");
  }
}

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

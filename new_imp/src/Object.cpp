/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Object.h"

namespace IMP
{

Object::Object()
{
  check_value_=111111111;
}

Object::~Object()
{
  assert_is_valid();
  check_value_=666666666;
}

void Object::assert_is_valid() const
{
  IMP_assert(check_value_==111111111,
             "Previously freed object is not valid: " << this);
}

} // namespace IMP

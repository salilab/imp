/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/internal/Object.h"

namespace IMP
{

namespace internal
{

Object::Object()
{
  check_value_=111111111;
  is_owned_=false;
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

} //namespace internal

} // namespace IMP

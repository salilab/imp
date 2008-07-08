/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Object.h"
#include "IMP/RefCountedObject.h"

namespace IMP
{

unsigned int RefCountedObject::live_objects_=0;

Object::Object()
{
  check_value_=111111111;
  count_=0;
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

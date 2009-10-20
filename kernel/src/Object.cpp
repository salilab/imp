/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Object.h"
#include "IMP/RefCounted.h"

IMP_BEGIN_NAMESPACE

unsigned int RefCounted::live_objects_=0;

RefCounted::~ RefCounted() {
  IMP_INTERNAL_CHECK(get_ref_count()== 0,
                     "Deleting object which still has references");
    IMP_LOG(MEMORY, "Deleting ref counted object " << this << std::endl);
    --live_objects_;
  }

Object::Object(std::string name): name_(name)
{
#if IMP_BUILD < IMP_FAST
  log_level_=DEFAULT;
  check_value_=111111111;
  was_owned_=false;
#endif
  IMP_LOG(MEMORY, "Creating object " << this << std::endl);
}

Object::~Object()
{
  IMP_OBJECT_LOG;
  IMP_INTERNAL_CHECK(get_is_valid(), "Object " << this << " previously freed.");
#if IMP_BUILD < IMP_FAST
  check_value_=666666666;
  if (!was_owned_) {
    IMP_WARN("Object \"" << get_name() << "\" was never owned."
             << " See the IMP::Object documentation for an explaination."
             << std::endl);
  }
#endif
  IMP_LOG(MEMORY, "Destroying object " << this << std::endl);
}

IMP_END_NAMESPACE

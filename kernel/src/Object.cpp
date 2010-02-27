/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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
  IMP_LOG(MEMORY, "Creating object \"" << name
          << "\" (" << this << ")" << std::endl);
}

Object::~Object()
{
  IMP_OBJECT_LOG;
  IMP_INTERNAL_CHECK(get_is_valid(), "Object " << this << " previously freed "
                     << "but something is trying to delete it again. Make sure "
                     << "that all C++ code uses IMP::Pointer objects to"
                     << " store it.");
#if IMP_BUILD < IMP_FAST
  check_value_=666666666;
  if (!was_owned_) {
    IMP_WARN("Object \"" << get_name() << "\" was never used."
             << " See the IMP::Object documentation for an explanation."
             << std::endl);
  }
#endif
  IMP_LOG(MEMORY, "Destroying object \"" << get_name() << "\" ("
          << this << ")" << std::endl);
}

IMP_END_NAMESPACE

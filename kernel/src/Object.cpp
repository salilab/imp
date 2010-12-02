/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Object.h"
#include "IMP/RefCounted.h"
#include "IMP/internal/map.h"
#include <boost/format.hpp>
#include "IMP/internal/static.h"

IMP_BEGIN_NAMESPACE

RefCounted::~ RefCounted() {
  IMP_INTERNAL_CHECK(get_ref_count()== 0,
                     "Deleting object which still has references");
#if IMP_BUILD < IMP_FAST
  --live_objects_;
#endif
}

Object::Object(std::string name)
{
#if IMP_BUILD < IMP_FAST
  log_level_=DEFAULT;
  check_value_=111111111;
  was_owned_=false;
#endif
  if (std::find(name.begin(), name.end(), '%') != name.end()) {
    if (internal::object_type_counts.find(name)
        == internal::object_type_counts.end()) {
      internal::object_type_counts[name]=0;
    }
    std::ostringstream oss;
    try {
      oss << boost::format(name)
        % internal::object_type_counts.find(name)->second;
    } catch(...) {
      IMP_THROW("Invalid format specified in name, should be %1%: "<< name,
                ValueException);
    }
    name_= oss.str();
    ++internal::object_type_counts.find(name)->second;
  } else {
    name_=name;
  }
  IMP_LOG(MEMORY, "Creating object \"" << name
          << "\" (" << this << ")" << std::endl);
}

Object::~Object()
{
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

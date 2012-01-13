/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/Object.h"
#include "IMP/base/RefCounted.h"
#include "IMP/compatibility/map.h"
#include <boost/format.hpp>
#include "IMP/base/internal/static.h"
#include <exception>

IMPBASE_BEGIN_NAMESPACE

RefCounted::~ RefCounted() {
  IMP_INTERNAL_CHECK(get_ref_count()== 0,
                     "Deleting object which still has references");
#if IMP_BUILD < IMP_FAST
  check_value_=666666666;
  --live_objects_;
#endif
}

Object::Object(std::string name)
{
#if IMP_BUILD < IMP_FAST
  log_level_=DEFAULT;
  check_level_=DEFAULT_CHECK;
  was_owned_=false;
  add_live_object(this);
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
  IMP_LOG(MEMORY, "Creating object \"" << name_
          << "\" (" << this << ")" << std::endl);
}

Object::~Object()
{
  IMP_INTERNAL_CHECK(get_is_valid(), "Object " << this << " previously freed "
                     << "but something is trying to delete it again. Make sure "
                     << "that all C++ code uses IMP::Pointer objects to"
                     << " store it.");
#if IMP_BUILD < IMP_FAST
  // if there is no exception currently being handled warn if it was not owned
  if (!was_owned_ && !std::uncaught_exception()) {
    IMP_WARN("Object \"" << get_name() << "\" was never used."
             << " See the IMP::Object documentation for an explanation."
             << std::endl);
  }
  remove_live_object(this);
#endif
  IMP_LOG(MEMORY, "Destroying object \"" << get_name() << "\" ("
          << this << ")" << std::endl);
  // cleanup
#if IMP_BUILD < IMP_FAST
  if (log_level_ != DEFAULT) {
    IMP::base::set_log_level(log_level_);
  }
#endif
}



IMPBASE_END_NAMESPACE

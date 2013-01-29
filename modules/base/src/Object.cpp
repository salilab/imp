/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/Object.h"
#include "IMP/base/RefCounted.h"
#include "IMP/base/log.h"
#include "IMP/base/exception.h"
#include "IMP/base/utility.h"
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
  name_= get_unique_name(name);
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
#if IMP_BUILD < IMP_FAST && !IMP_BASE_HAS_LOG4CXX
  if (log_level_ != DEFAULT) {
    IMP::base::set_log_level(log_level_);
  }
#endif
}

void Object::_on_destruction() {
  // this can cause problems with the libs being unloaded in the wrong order
#if IMP_BUILD < IMP_FAST && !IMP_BASE_HAS_LOG4CXX
  LogLevel old=IMP::base::get_log_level();
  if (log_level_!= DEFAULT) {
    IMP::base::set_log_level(log_level_);
  }
  log_level_=old;
#endif
}

void Object::set_log_level(LogLevel l) {
  IMP_CHECK_VARIABLE(l);
  IMP_USAGE_CHECK(l <= MEMORY && l >= DEFAULT, "Setting to invalid log level "
                  << l);
#if IMP_BUILD < IMP_FAST
  log_level_=l;
#endif
}


void Object::show(std::ostream &out) const {
  out << get_name() << "\n";
}


IMPBASE_END_NAMESPACE

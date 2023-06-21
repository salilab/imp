/**
 *  \file Object.cpp
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Object.h"
#include "IMP/log.h"
#include "IMP/log_macros.h"
#include "IMP/exception.h"
#include "IMP/utility.h"
#include "IMP/internal/utility.h"
#include <cereal/types/string.hpp>
#include <exception>

IMPKERNEL_BEGIN_NAMESPACE

Object::Object(std::string name) { initialize(name); }

void Object::initialize(std::string name) {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  ++live_objects_;
#endif
#if IMP_HAS_CHECKS >= IMP_USAGE
  check_value_ = 111111111;
#endif
  count_ = 0;

#if IMP_HAS_LOG >= IMP_PROGRESS
  log_level_ = DEFAULT;
#endif
#if IMP_HAS_CHECKS >= IMP_USAGE
  check_level_ = DEFAULT_CHECK;
  was_owned_ = false;
#endif
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  add_live_object(this);
#endif
  set_name(name);
  IMP_LOG_MEMORY("Creating object \"" << get_name() << "\" (" << this << ")"
                                      << std::endl);
}

Object::~Object() {
  IMP_INTERNAL_CHECK(
      get_is_valid(),
      "Object " << this << " previously freed "
                << "but something is trying to delete it again. Make sure "
                << "that all C++ code uses IMP::Pointer objects to"
                << " store it.");
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  // if there is no exception currently being handled warn if it was not owned
  if (!was_owned_ && !std::uncaught_exception()) {
    IMP_WARN(
        "Object \"" << get_name() << "\" was never used."
                    << " See the IMP::Object documentation for an explanation."
                    << std::endl);
  }
#endif
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  remove_live_object(this);
#endif
  IMP_LOG_MEMORY("Destroying object \"" << get_name() << "\" (" << this << ")"
                                        << std::endl);
#if IMP_HAS_LOG > IMP_NONE && !IMP_KERNEL_HAS_LOG4CXX
  // cleanup
  if (log_level_ != DEFAULT) {
    IMP::set_log_level(log_level_);
  }
#endif

  IMP_INTERNAL_CHECK(get_ref_count() == 0,
                     "Deleting object which still has references");
#if IMP_HAS_CHECKS >= IMP_USAGE
  check_value_ = 666666666;
#endif
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  --live_objects_;
#endif
}

void Object::_on_destruction() {
// this can cause problems with the libs being unloaded in the wrong order
#if IMP_HAS_LOG != IMP_SILENT && !IMP_KERNEL_HAS_LOG4CXX
  LogLevel old = IMP::get_log_level();
  if (log_level_ != DEFAULT) {
    IMP::set_log_level(log_level_);
  }
  log_level_ = old;
#endif
  do_destroy();
}

void Object::set_log_level(LogLevel l) {
  IMP_CHECK_VARIABLE(l);
  IMP_USAGE_CHECK(l <= MEMORY && l >= DEFAULT, "Setting to invalid log level "
                                                   << l);
#if IMP_HAS_LOG != IMP_SILENT
  if (l != log_level_) {
    log_level_ = l;
  }
#endif
}

void Object::set_name(std::string in_name) {
  set_name_internal(get_unique_name(in_name));
}

void Object::set_name_internal(std::string in_name) {
  name_ = in_name;
  quoted_name_.reset(new char[name_.size() + 3]);
  quoted_name_[0] = '"';
  std::copy(name_.begin(), name_.end(), quoted_name_.get() + 1);
  quoted_name_[name_.size() + 1] = '"';
  quoted_name_[name_.size() + 2] = '\0';
}

void Object::show(std::ostream &out) const {
  out << "\"" << get_name() << "\"";
}

void Object::unref() const {
  IMP_INTERNAL_CHECK(count_ != 0, "Too many unrefs on object");
  IMP_LOG_MEMORY("Unrefing object \"" << get_name() << "\" (" << count_ << ") {"
                                      << this << "}" << std::endl);
  --count_;
  if (count_ == 0) {
    delete this;
  }
}

void Object::release() const {
  IMP_INTERNAL_CHECK(count_ != 0, "Release called on unowned object");
  --count_;
  IMP_LOG_MEMORY("Releasing object \"" << get_name() << "\" (" << count_
                                       << ") {" << this << "}" << std::endl);
}

std::map<std::string, Object::OutputSerializer> &
Object::get_output_serializers() {
  static std::map<std::string, OutputSerializer> m;
  return m;
}

std::map<std::string, Object::LoadFunc> &
Object::get_input_serializers() {
  static std::map<std::string, LoadFunc> m;
  return m;
}

bool Object::register_serialize(const std::type_info &t, std::string name,
                                SaveFunc save_func, LoadFunc load_func) {
  if (name.substr(0, 5) != "IMP::") {
    IMP_THROW("Class name for register_serialize is not fully qualified ("
              << name << "); should start with IMP::", ValueException);
  }
  OutputSerializer os;
  os.class_name = name;
  os.save_func = save_func;
  get_output_serializers()[t.name()] = os;
  get_input_serializers()[name] = load_func;
  return true;
}

void Object::poly_serialize(cereal::BinaryOutputArchive &ar) {
  const std::type_info &oi = typeid(*this);
  std::map<std::string, OutputSerializer>
          &ti_to_name = get_output_serializers();
  auto f = ti_to_name.find(oi.name());
  if (f == ti_to_name.end()) {
    IMP_THROW("Trying to save an unregistered polymorphic type ("
              << internal::demangle_cxx(oi.name())
              << "), probably a missing IMP_OBJECT_SERIALIZE_IMPL.",
              TypeException);
  } else {
    ar(f->second.class_name);
    f->second.save_func(this, ar);
  }
}

Object *Object::poly_unserialize(cereal::BinaryInputArchive &ar) {
  std::string class_name;
  ar(class_name);
  std::map<std::string, LoadFunc> &ti_to_f = get_input_serializers();
  auto f = ti_to_f.find(class_name);
  if (f == ti_to_f.end()) {
    IMP_THROW("Trying to load an unregistered polymorphic type ("
              << class_name <<
              "), perhaps because its module has not been loaded yet.",
              TypeException);
  } else {
    return f->second(ar);
  }
}

IMPKERNEL_END_NAMESPACE

/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/exceptions.h>
#include <sstream>
#include <algorithm>

namespace RMF {
Exception::~Exception() throw() {}
const char *Exception::what() const throw() {
  static const int buf_size=10000;
  static char buffer[buf_size]={};
  try {
    using std::operator<<;
    std::ostringstream oss;
    oss << "A " << get_type() << " error occurred: ";
    oss << '"' << message_ << '"';
    if (!operation_.empty()) {
      oss << " on " << operation_;
    }
    if (!file_name_.empty()) {
      oss << ". File is \"" << file_name_ << "\"";
    }
    std::string str= oss.str();
    std::copy(str.begin(),
              str.begin()+std::min<int>(oss.str().size(), buf_size-1),
              buffer);
  } catch (...) {
  }
  return buffer;
}

  void Exception::set_operation_name(const char *name) throw() {
    try {
      operation_=name;
    } catch (...){}
  }
  void Exception::set_file_name(const char *name) throw() {
    try {
      file_name_=name;
    } catch (...){}
  }
UsageException::UsageException(const char *msg): Exception(msg){}
UsageException::~UsageException() throw() {}
const char *UsageException::get_type() const {
  static const char *name="usage";
  return name;
}

IOException::IOException(const char *msg): Exception(msg){}
IOException::~IOException() throw() {}
const char *IOException::get_type() const {
  static const char *name="IO";
  return name;
}
InternalException::InternalException(const char *msg): Exception(msg){}

InternalException::~InternalException() throw() {}
const char *InternalException::get_type() const {
  static const char *name="Internal";
  return name;
}

} /* namespace RMF */

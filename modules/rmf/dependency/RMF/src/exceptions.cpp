/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <sstream>
#include <string>

#include "RMF/ID.h"
#include "RMF/compiler_macros.h"
#include "RMF/exceptions.h"
#include "RMF/internal/errors.h"

RMF_ENABLE_WARNINGS

namespace RMF {
Exception::Exception() {}

const char* Exception::what() const noexcept {
  try {
    if (message_.empty()) {
      message_ = get_message(*this);
    }
  }
  catch (...) {
  }
  return message_.c_str();
}

Exception::~Exception() noexcept {}
std::string get_message(const Exception& e) {
  using namespace RMF::internal::ErrorInfo;
  try {
    std::ostringstream oss;
    const std::string* type = boost::get_error_info<Type>(e);
    if (type) {
      oss << *type << "Error:";
    }
    const std::string* expression = boost::get_error_info<Expression>(e);
    if (expression) {
      oss << " " << *expression;
    }
    const std::string* message = boost::get_error_info<Message>(e);
    if (message) {
      oss << " \"" << *message << "\"";
    }
    const std::string* operation = boost::get_error_info<Operation>(e);
    if (operation) {
      oss << " while " << *operation;
    }
    const std::string* component = boost::get_error_info<Component>(e);
    if (component) {
      oss << " component \"" << *component << "\"";
    }
    const std::string* file = boost::get_error_info<File>(e);
    if (file) {
      oss << " in file \"" << *file << "\"";
    }
    const FrameID* frame = boost::get_error_info<Frame>(e);
    if (frame) {
      oss << " at frame " << *frame;
    }
    const NodeID* node = boost::get_error_info<Node>(e);
    if (node) {
      oss << " at node " << *node;
    }
    const std::string* key = boost::get_error_info<Key>(e);
    if (key) {
      oss << " processing key \"" << *key << "\"";
    }
    const std::string* category =
        boost::get_error_info<internal::ErrorInfo::Category>(e);
    if (category) {
      oss << " processing category \"" << *category << "\"";
    }
    const std::string* decorator = boost::get_error_info<Decorator>(e);
    if (decorator) {
      oss << " processing decorator of type " << *decorator;
    }
    const std::string* source = boost::get_error_info<SourceFile>(e);
    if (source) {
      oss << " at " << *source << ":" << *boost::get_error_info<SourceLine>(e);
    }
    const std::string* function = boost::get_error_info<Function>(e);
    if (function) {
      oss << " in " << *function;
    }
    return oss.str();
  }
  catch (...) {
    return "Error formatting message.";
  }
}
UsageException::UsageException() : Exception() {}
UsageException::~UsageException() noexcept {}

IOException::IOException() : Exception() {}
IOException::~IOException() noexcept {}

IndexException::IndexException() : Exception() {}
IndexException::~IndexException() noexcept {}

InternalException::InternalException() : Exception() {}
InternalException::~InternalException() noexcept {}

} /* namespace RMF */

RMF_DISABLE_WARNINGS

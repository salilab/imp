/**
 *  \file RMF/names.cpp
 *  \brief Functions to check if names are valid.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/names.h"

#include <boost/algorithm/string/erase.hpp>
#include <string>

#include "RMF/compiler_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {
extern std::string rmf_data_path;
extern std::string rmf_example_path;
}

std::string get_as_node_name(std::string in) {
  boost::erase_all(in, "\"");
  return in;
}

bool get_is_valid_node_name(std::string name) {
  if (name.empty()) {
    return false;
  }
  static const char* illegal = "\"";
  const char* cur = illegal;
  while (*cur != '\0') {
    if (name.find(*cur) != std::string::npos) {
      return false;
    }
    ++cur;
  }
  return true;
}

bool get_is_valid_key_name(std::string name) {
  if (name.empty()) {
    return false;
  }
  static const char* illegal = "\\:=()[]{}\"'";
  const char* cur = illegal;
  while (*cur != '\0') {
    if (name.find(*cur) != std::string::npos) {
      return false;
    }
    ++cur;
  }
  if (name.find("  ") != std::string::npos) {
    return false;
  }
  return true;
}
} /* namespace RMF */

RMF_DISABLE_WARNINGS

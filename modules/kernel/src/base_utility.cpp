/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base_utility.h"
#include <boost/unordered_map.hpp>
#include <boost/format.hpp>
#include "internal/base_static.h"
IMPKERNEL_BEGIN_NAMESPACE
std::string get_unique_name(std::string name) {
  if (std::find(name.begin(), name.end(), '%') != name.end()) {
    if (internal::object_type_counts.find(name) ==
        internal::object_type_counts.end()) {
      internal::object_type_counts[name] = 0;
    }
    std::ostringstream oss;
    try {
      oss << boost::format(name) %
                 internal::object_type_counts.find(name)->second;
    }
    catch (...) {
      IMP_THROW("Invalid format specified in name, should be %1%: " << name,
                ValueException);
    }
    ++internal::object_type_counts.find(name)->second;
    name = oss.str();
    return name;
  } else {
    return name;
  }
}

std::string get_copyright() {
  return "Copyright 2007-2023 IMP Inventors";
}

IMPKERNEL_END_NAMESPACE

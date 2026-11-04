/**
 *  \file utility.cpp
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/utility.h>
#include <boost/unordered_map.hpp>
#include <boost/format.hpp>
#include "internal/static.h"

IMPKERNEL_BEGIN_NAMESPACE

std::string get_unique_name(std::string name) {
  std::size_t pos = name.find('%');
  if (pos == std::string::npos) {
    return name;
  }
  unsigned int &count = internal::object_type_counts[name];
  if (name.compare(pos, 3, "%1%") == 0
      && name.find('%', pos + 3) == std::string::npos) {
    // Handle the most common format directly, since this function is called
    // for every new Particle and boost::format is expensive
    name.replace(pos, 3, std::to_string(count));
  } else {
    std::ostringstream oss;
    try {
      oss << boost::format(name) % count;
    }
    catch (...) {
      IMP_THROW("Invalid format specified in name, should be %1%: " << name,
                ValueException);
    }
    name = oss.str();
  }
  ++count;
  return name;
}

std::string get_copyright() {
  return "Copyright 2007-2026 IMP Inventors";
}

IMPKERNEL_END_NAMESPACE

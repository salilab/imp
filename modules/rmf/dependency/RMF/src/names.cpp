/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/names.h>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fstream>

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
} /* namespace RMF */

RMF_DISABLE_WARNINGS

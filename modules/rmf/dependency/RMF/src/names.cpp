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

namespace RMF {
namespace internal {
extern std::string rmf_data_path;
extern std::string rmf_example_path;
}
namespace {
std::string get_concatenated_path(std::string part0,
                                  std::string part1) {
  return part0 + "/" + part1;
}
std::string path_cat(std::string base,
                     std::string file_name) {
  if (file_name.empty() || file_name[0] == '/') {
    RMF_THROW(Message(std::string("File name should be relative to the RMF")
                      + std::string(" directory and non-empty, not ")
                      + file_name),
              UsageException);
  }
  std::string ret = get_concatenated_path(base, file_name);
  return ret;
}
std::string get_path(std::string envvar,
                     std::string def, std::string file_name) {
  char *env = getenv(envvar.c_str());
  std::string base;
  using std::operator<<;
  if (env) {
    base = std::string(env);
    base = get_concatenated_path(base, "RMF");
  } else {
    // Default to compiled-in value
    base = def;
  }
  std::string ret = path_cat(base, file_name);
  return ret;
}
}
std::string get_example_path(std::string file_name)
{
  std::string varname = std::string("RMF_EXAMPLE_DATA");
  std::string path = get_path(varname,
                              internal::rmf_example_path + "/RMF", file_name);
  std::ifstream in(path.c_str());
  if (!in) {
    RMF_THROW(Message(std::string("Unable to find example file ")
                      + file_name + std::string(" at ")
                      + path
                      + std::string(". RMF is not installed or set up correctly.")),
              UsageException);
  }
  return path;
}

std::string get_as_node_name(std::string in) {
  boost::erase_all(in, "\"");
  return in;
}
} /* namespace RMF */

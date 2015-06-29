/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#include <string>

#include "RMF/compiler_macros.h"
#include "RMF/internal/paths.h"

RMF_ENABLE_WARNINGS
namespace RMF {
namespace internal {
std::string get_file_name(std::string path) {
#if BOOST_VERSION >= 104600
  return boost::filesystem::path(path).filename().string();
#else
  return boost::filesystem::path(path).filename();
#endif
}

std::string get_relative_path(std::string /*base*/, std::string file) {
  // assume it already is
  return file;
}
std::string get_absolute_path(std::string base, std::string file) {

#if BOOST_VERSION >= 104800
  boost::filesystem::path parent = boost::filesystem::path(base).parent_path();
  return boost::filesystem::canonical(boost::filesystem::path(parent) /
                                      boost::filesystem::path(file)).string();
#else
  boost::filesystem::path parent = boost::filesystem::path(base).parent_path();
  return (boost::filesystem::path(parent) / boost::filesystem::path(file))
      .string();
#endif
}

std::string get_unique_path() {
#if BOOST_VERSION > 104100
  boost::filesystem::path temp = boost::filesystem::unique_path();
  return temp.string();
#else
  return tmpnam(NULL);
#endif
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

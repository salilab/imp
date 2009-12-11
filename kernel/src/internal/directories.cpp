/**
 *  \file directories.cpp
 *  \brief Get directories used by IMP.
 *  \note  Auto-generated from directories.cpp.in; do not edit.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/internal/directories.h>
#include <IMP/config.h>

#ifdef IMP_USE_BOOST_LIBS
#include <boost/filesystem.hpp>
#endif

#include <cstdlib>

IMP_BEGIN_INTERNAL_NAMESPACE

namespace {
  std::string path_cat(std::string base, std::string module,
                       std::string file_name) {
    boost::filesystem::path basep(base);
    if (module != "kernel") {
#ifdef IMP_USE_BOOST_LIBS
      return (basep/module/file_name).file_string();
#else
      return basep+std::string("/")+module+std::string("/")+file_name;
#endif
    } else {
#ifdef IMP_USE_BOOST_LIBS
      return (basep/file_name).file_string();
#else
      return basep+std::string("/")+file_name;
#endif
    }
  }
  std::string get_path(std::string envvar, std::string def,
                       std::string module, std::string file_name) {
    char *env = getenv(envvar.c_str());
    std::string base;
    if (env) {
      base=env;
    } else {
      // Default to compiled-in value
      base=def;
    }
    return path_cat(base, module, file_name);
  }
}

std::string get_data_path(std::string module, std::string file_name)
{
  return get_path("IMP_DATA_DIRECTORY", IMP_DATA_DIRECTORY,
                  module, file_name);
}
std::string get_example_path(std::string module, std::string file_name)
{
  return get_path("IMP_EXAMPLE_DIRECTORY", IMP_EXAMPLE_DIRECTORY,
                  module, file_name);
}

IMP_END_INTERNAL_NAMESPACE

/**
 *  \file directories.cpp
 *  \brief Get directories used by IMP.
 *  \note  Auto-generated from directories.cpp.in; do not edit.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/internal/directories.h>
#include <IMP/config.h>
#include <IMP/exception.h>
#include <fstream>

#ifdef IMP_USE_BOOST_LIBS
#include <boost/filesystem/path.hpp>
#endif

#include <cstdlib>

IMP_BEGIN_INTERNAL_NAMESPACE

namespace {
#ifdef IMP_USE_BOOST_LIBS
  std::string to_string(boost::filesystem::path path) {
#if BOOST_VERSION >= 103400
    return path.file_string();
#else
    return path.native_file_string();
#endif
  }
#endif

  std::string path_cat(std::string base, std::string module,
                       std::string file_name) {
#ifdef IMP_USE_BOOST_LIBS
    boost::filesystem::path basep(base);
#endif
    if (module != "kernel") {
#ifdef IMP_USE_BOOST_LIBS
      return to_string(basep/module/file_name);
#else
      return base+"/"+module+"/"+file_name;
#endif
    } else {
#ifdef IMP_USE_BOOST_LIBS
      return to_string(basep/file_name);
#else
      return base+"/"+file_name;
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
  std::string path= get_path("IMP_DATA_DIRECTORY", IMP_DATA_DIRECTORY,
                             module, file_name);
  std::ifstream in(path.c_str());
  if (!in) {
    IMP_FAILURE("Unable to find data file "
                << file_name << " at " << path);
  }
  return path;
}
std::string get_example_path(std::string module, std::string file_name)
{
  std::string path= get_path("IMP_EXAMPLE_DIRECTORY", IMP_EXAMPLE_DIRECTORY,
                             module, file_name);
  std::ifstream in(path.c_str());
  if (!in) {
    IMP_FAILURE("Unable to find example file "
                << file_name << " at " << path);
  }
  return path;
}

IMP_END_INTERNAL_NAMESPACE

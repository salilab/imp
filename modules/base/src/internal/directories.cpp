/**
 *  \file directories.cpp
 *  \brief Get directories used by IMP.
 *  \note @AUTOGEN@
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/internal/directories.h>
#include <IMP/base/exception.h>
#include <fstream>
#include <boost/algorithm/string.hpp>

#ifdef IMP_KERNEL_USE_BOOST_FILESYSTEM
#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#endif

#include <cstdlib>

IMPBASE_BEGIN_INTERNAL_NAMESPACE

namespace {
/** Boost versions older than 1.35 cannot handle "hidden" paths,
    e.g. /etc/skel/.bashrc; see https://svn.boost.org/trac/boost/ticket/1378
 */
#if defined(IMP_KERNEL_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
  std::string to_string(boost::filesystem::path path) {
    return path.file_string();
  }
#else
  const std::string& to_string(const std::string& s) {return s;}
#endif
}
std::string get_concatenated_path(std::string part0,
                                   std::string part1) {
#if defined(IMP_KERNEL_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
  boost::filesystem::path b0(part0), b1(part1);
  return to_string(b0/b1);
#else
  return to_string(part0+"/"+part1);
#endif
}

namespace {
  std::string backup_search_path;

  std::string path_cat(std::string base, std::string module,
                       std::string file_name) {
    IMP_USAGE_CHECK(!file_name.empty() && file_name[0] != '/',
                    "File name should be relative to the IMP directory and"
                    << " non-empty, not " << file_name);
    if (module != "kernel") {
      return get_concatenated_path(base,
                                   get_concatenated_path(module, file_name));
    } else {
      return get_concatenated_path(base, file_name);
    }
  }

  std::string get_path(std::string envvar,
                       std::string def,
                       std::string module, std::string file_name) {
    char *env = getenv(envvar.c_str());
    std::string base;
    if (env) {
      base=std::string(env);
    } else {
      // Default to compiled-in value
      base=def;
    }
    return path_cat(base, module, file_name);
  }
}

void set_backup_data_path(std::string path) {
  // should check that it is a valid path
  backup_search_path=path;
}

namespace {
  bool get_path_exists(std::string name) {
#if defined(IMP_KERNEL_USE_BOOST_FILESYSTEM)
    return boost::filesystem::exists(name);
#else
    // does not work on binary files
    std::ifstream in(name.c_str(), std::ios::binary);
    return in;
#endif
  }
}


std::string get_directory_path(std::string fileordirectory) {
#if defined(IMP_KERNEL_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
  try {
    boost::filesystem::path fnp(fileordirectory);
    boost::filesystem::path dir=
#if BOOST_VERSION >= 103600
      fnp.remove_filename();
#else
      fnp.branch_path();
#endif
    return to_string(dir);
  } catch (boost::filesystem::filesystem_error e) {
    IMP_THROW("Error splitting file name \""
              << fileordirectory
              << "\" got " << e.what(), IOException);
  }
#else
  for (int i = fileordirectory.size()-1; i>=0; --i) {
    if (fileordirectory[i] == '/'
        || fileordirectory[i] == '\\') {
      return std::string(fileordirectory, 0, i);
    }
  }
  return std::string();
#endif
}



extern std::string imp_data_path;
extern std::string imp_example_path;

std::string get_data_path(std::string module, std::string file_name)
{
  std::string varname=std::string("IMP_")+boost::to_upper_copy(module)
    +std::string("_DATA");
  std::string path= get_path(varname,
                                   imp_data_path,
                                   module, file_name);
  {
    if (get_path_exists(path)) {
      return path;
    }
  }
#if defined(IMP_KERNEL_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
  if (!backup_search_path.empty()) {
    boost::filesystem::path path
      = boost::filesystem::path(backup_search_path)/file_name;
    if (get_path_exists(path.native_file_string())) {
      return path.native_file_string();
    }
  }
#endif
  IMP_THROW("Unable to find data file "
            << file_name << " at " << path
            << ". IMP is not installed or set up correctly.",
            IOException);
}
std::string get_example_path(std::string module, std::string file_name)
{
  std::string varname=std::string("IMP_")+boost::to_upper_copy(module)
    +std::string("_EXAMPLE_DATA");
  std::string path= get_path(varname,
                                   imp_example_path,
                                   module, file_name);
  std::ifstream in(path.c_str());
  if (!in) {
    IMP_THROW("Unable to find example file "
              << file_name << " at " << path
              << ". IMP is not installed or set up correctly.",
              IOException);
  }
  return path;
}



IMPBASE_END_INTERNAL_NAMESPACE

/**
 *  \file directories.cpp
 *  \brief Get directories used by IMP.
 *  \note @AUTOGEN@
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/internal/directories.h>
#include <IMP/base/exception.h>
#include <fstream>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/iterator/iterator_facade.hpp>

#ifdef IMP_BASE_USE_BOOST_FILESYSTEM
#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#endif

#include <cstdlib>

#ifdef _MSC_VER
#include <windows.h>

// Registry functions are not in kernel but advapi32.dll
#pragma comment(lib,"advapi32")
#endif

IMPBASE_BEGIN_INTERNAL_NAMESPACE

namespace {

#ifdef _MSC_VER
// assume IMP version is the IMP::base version without any 'with' suffix
std::string get_imp_version() {
  std::string full_version = get_module_version();
  size_t pos = full_version.find(" with");
  if (pos != std::string::npos) {
    full_version.resize(pos);
  }
  return full_version;
}

/** Get the directory where IMP is installed, by reading the
    Windows registry key (set by the IMP binary installer).
    \return true iff the registry key was found.
 */
bool get_install_location(std::string &dir) {
  // Only probe the registry once, then cache the value
  static bool called, ret;
  static std::string cached_dir;
  if (called) {
    if (ret) {
      dir.assign(cached_dir);
    }
    return ret;
  }

  std::string version = get_imp_version();
  std::string regkey = "SOFTWARE\\IMP-" + version;
  HKEY key;
  ret = false;

  if (RegOpenKeyEx(HKEY_CURRENT_USER, regkey.c_str(), 0, KEY_READ,
                   &key) == ERROR_SUCCESS) {
    DWORD keylen, keytype;
    if (RegQueryValueEx(key, "", NULL,
        &keytype, NULL, &keylen) == ERROR_SUCCESS && keytype == REG_SZ) {
      char *keyval = new char[keylen];

      if (RegQueryValueEx(key, "", NULL,
          &keytype, (LPBYTE)keyval, &keylen) == ERROR_SUCCESS) {
        dir.assign(keyval);
        cached_dir.assign(keyval);
        ret = true;
      }
      delete[] keyval;
    }
    RegCloseKey(key);
  }
  called = true;
  return ret;
}
#endif

/** Boost versions older than 1.35 cannot handle "hidden" paths,
    e.g. /etc/skel/.bashrc; see https://svn.boost.org/trac/boost/ticket/1378
 */
#if defined(IMP_BASE_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
  std::string to_string(boost::filesystem::path path) {
    return path.file_string();
  }
#else
  const std::string& to_string(const std::string& s) {return s;}
#endif
}
std::string get_concatenated_path(std::string part0,
                                   std::string part1) {
#if defined(IMP_BASE_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
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
      std::string ret= get_concatenated_path(base,
                                   get_concatenated_path(module, file_name));
      return ret;
    } else {
      return get_concatenated_path(base, file_name);
    }
  }

  std::string get_path(std::string envvar,
#ifdef _MSC_VER
                       std::string install_subdir,
#endif
                       std::string def,
                       std::string module, std::string file_name) {
    char *env = getenv(envvar.c_str());
    std::string base;
    if (env) {
      base=std::string(env);
    } else {
#ifdef _MSC_VER
      // Since on Windows the IMP binary package is relocatable (the user
      // can install it in any location), try this install location if available
      // (the binary installer stores the location in the Windows registry)
      std::string install_location;
      if (get_install_location(install_location)) {
        base = install_location + "\\" + install_subdir;
      } else {
        base=def;
      }
#else
      // Default to compiled-in value
      base=def;
#endif
    }
    std::string ret= path_cat(base, module, file_name);
    return ret;
  }
}

void set_backup_data_path(std::string path) {
  // should check that it is a valid path
  backup_search_path=path;
}

namespace {
  bool get_path_exists(std::string name) {
#if defined(IMP_BASE_USE_BOOST_FILESYSTEM)
    return boost::filesystem::exists(name);
#else
    // does not work on binary files
    std::ifstream in(name.c_str(), std::ios::binary);
    return in;
#endif
  }
}


std::string get_directory_path(std::string fileordirectory) {
#if defined(IMP_BASE_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
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
#ifdef _MSC_VER
                             "data",
#endif
                             imp_data_path, module, file_name);
  {
    if (get_path_exists(path)) {
      return path;
    }
  }
#if defined(IMP_BASE_USE_BOOST_FILESYSTEM) && BOOST_VERSION >= 103500
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
#ifdef _MSC_VER
                             "examples",
#endif
                             imp_example_path, module, file_name);
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

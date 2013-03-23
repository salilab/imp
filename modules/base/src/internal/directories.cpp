/**
 *  \file directories.cpp
 *  \brief Get directories used by IMP.
 *  \note @AUTOGEN@
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/internal/directories.h>
#include <IMP/base/exception.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/warning_macros.h>
#include <IMP/base/types.h>
#include <fstream>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION >= 104600 && !defined(BOOST_FILESYSTEM_VERSION)
#  define BOOST_FILESYSTEM_VERSION 3
#else
#  define BOOST_FILESYSTEM_VERSION 2
#endif
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdlib>

#ifdef _MSC_VER
#include <windows.h>

// Registry functions are not in kernel but advapi32.dll
#pragma comment(lib,"advapi32")
#endif

IMPBASE_BEGIN_INTERNAL_NAMESPACE

extern std::string imp_data_path;
extern std::string imp_example_path;

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
    if (RegQueryValueEx(key, "", nullptr,
        &keytype, nullptr, &keylen) == ERROR_SUCCESS && keytype == REG_SZ) {
      char *keyval = new char[keylen];

      if (RegQueryValueEx(key, "", nullptr,
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

  std::string backup_search_path;

  Strings get_data_prefixes(std::string ) {
    Strings ret;
    {
      char *env = getenv("IMP_DATA");
      if (env) {
        Strings cur;
        boost::split(cur, env, boost::is_any_of(":"));
        ret+=cur;
      }
    }
    {
#ifdef _MSC_VER
      // Since on Windows the IMP binary package is relocatable (the user
      // can install it in any location), try this install location if available
      // (the binary installer stores the location in the Windows registry)
      std::string install_location;
      if (get_install_location(install_location)) {
        ret.push_back(install_location+"\\data");
      }
#endif
    }
    ret.push_back(imp_data_path);
    if (!backup_search_path.empty()) {
      ret.push_back(backup_search_path);
    }
    return ret;
  }

  Strings get_example_prefixes(std::string ) {
    Strings ret;
    {
      char *env = getenv("IMP_EXAMPLE_DATA");
      if (env) {
        Strings cur;
        boost::split(cur, env, boost::is_any_of(":"));
        ret+=cur;
      }
    }
    {
#ifdef _MSC_VER
      // Since on Windows the IMP binary package is relocatable (the user
      // can install it in any location), try this install location if available
      // (the binary installer stores the location in the Windows registry)
      std::string install_location;
      if (get_install_location(install_location)) {
        ret.push_back(install_location+"\\examples");
      }
#endif
    }
    ret.push_back(imp_example_path);
    if (!backup_search_path.empty()) {
      ret.push_back(backup_search_path);
    }
    return ret;
  }

}

void set_backup_data_path(std::string path) {
  // should check that it is a valid path
  backup_search_path=path;
}


std::string get_directory_path(std::string fileordirectory) {
  try {
    boost::filesystem::path fnp(fileordirectory);
    boost::filesystem::path dir=
      fnp.remove_filename();
#if BOOST_FILESYSTEM_VERSION == 3
      return dir.string();
#else
      return dir.native_file_string();
#endif
  } catch (boost::filesystem::filesystem_error e) {
    IMP_THROW("Error splitting file name \""
              << fileordirectory
              << "\" got " << e.what(), IOException);
  }
}


std::string get_data_path(std::string module, std::string file_name)
{
  Strings prefixes=get_data_prefixes(module);
  for (unsigned int i = 0; i < prefixes.size(); ++i) {
    boost::filesystem::path path
      = boost::filesystem::path(prefixes[i])/module/file_name;
    if (boost::filesystem::exists(path)) {
#if BOOST_FILESYSTEM_VERSION == 3
      return path.string();
#else
      return path.native_file_string();
#endif
    }
  }
  IMP_THROW("Unable to find data file "
            << file_name << " in " << prefixes
            << ". IMP is not installed or set up correctly.",
            IOException);
}

std::string get_example_path(std::string module, std::string file_name)
{
  Strings prefixes=get_example_prefixes(module);
  for (unsigned int i = 0; i < prefixes.size(); ++i) {
    boost::filesystem::path path
      = boost::filesystem::path(prefixes[i])/module/file_name;
    if (boost::filesystem::exists(path)) {
#if BOOST_FILESYSTEM_VERSION == 3
      return path.string();
#else
      return path.native_file_string();
#endif
    }
  }
  IMP_THROW("Unable to find example file "
            << file_name << " in " << prefixes
            << ". IMP is not installed or set up correctly.",
            IOException);
}

std::string get_file_name(std::string path) {
#if BOOST_FILESYSTEM_VERSION == 3
  return boost::filesystem::path(path).filename().string();
#else
  return boost::filesystem::path(path).filename();
#endif
}


std::string get_concatenated_path(std::string part0,
                                  std::string part1) {
  boost::filesystem::path b0(part0), b1(part1);
#if BOOST_FILESYSTEM_VERSION == 3
  return (b0/b1).string();
#else
  return (b0/b1).native_file_string();
#endif
}

IMPBASE_END_INTERNAL_NAMESPACE

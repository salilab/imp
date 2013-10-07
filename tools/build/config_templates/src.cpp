/**
 *  \file config.cpp
 *  \brief base module version information.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <%(filename)s>
#include <IMP/base/internal/directories.h>

%(cppprefix)s_BEGIN_NAMESPACE

std::string get_module_version() {
  static std::string version("%(version)s");
  return version;
}
std::string get_data_path(std::string file_name) {
  return IMP::base::internal::get_data_path("%(name)s", file_name);
}
std::string get_example_path(std::string file_name)  {
  return IMP::base::internal::get_example_path("%(name)s", file_name);
}

%(cppprefix)s_END_NAMESPACE

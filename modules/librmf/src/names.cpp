/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/names.h>
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
      return part0+"/"+part1;
    }
    std::string path_cat(std::string base,
                         std::string file_name) {
      if (file_name.empty() || file_name[0] == '/') {
        IMP_RMF_THROW("File name should be relative to the RMF directory and"
                      << " non-empty, not " << file_name,
                      std::runtime_error);
      }
      std::string ret= get_concatenated_path(base, file_name);
      return ret;
    }
    std::string get_path(std::string envvar,
                         std::string def,std::string file_name) {
      char *env = getenv(envvar.c_str());
      std::string base;
      if (env) {
        base=std::string(env);
        base=get_concatenated_path(base, "RMF");
        std::cout << "found env " << base << std::endl;
      } else {
        // Default to compiled-in value
        base=def;
        std::cout << "using default base " << base << std::endl;
      }
      std::string ret= path_cat(base, file_name);
      return ret;
    }
  }
  std::string get_example_path(std::string file_name)
  {
    std::string varname=std::string("RMF_EXAMPLE_DATA");
    std::string path= get_path(varname,
                               internal::rmf_example_path+"/RMF", file_name);
    std::ifstream in(path.c_str());
    if (!in) {
      IMP_RMF_THROW("Unable to find example file "
                    << file_name << " at " << path
                    << ". RMF is not installed or set up correctly.",
                    std::runtime_error);
    }
    return path;
  }
} /* namespace RMF */

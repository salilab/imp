/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/paths.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <stdexcept>

RMF_ENABLE_WARNINGS

namespace RMF {
  namespace internal {
    std::string get_relative_path(std::string /*base*/,
                                  std::string file) {
      // assume it already is
      return file;
    }
    std::string get_absolute_path(std::string base,
                                  std::string file) {

#if BOOST_VERSION >= 104800
      boost::filesystem::path parent
        = boost::filesystem::path(base).parent_path();
      return boost::filesystem::canonical(boost::filesystem::path(parent)
                                          /boost::filesystem::path(file))
        .string();
#else
      boost::filesystem::path parent
        = boost::filesystem::path(base).parent_path();
      return (boost::filesystem::path(parent)/boost::filesystem::path(file))
        .string();
#endif
    }
  } // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

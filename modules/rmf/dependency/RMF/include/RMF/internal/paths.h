/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_PATHS_H
#define RMF_INTERNAL_PATHS_H

#include "RMF/config.h"
#include <string>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {
RMFEXPORT std::string get_file_name(std::string file);

// Get a relative path to file from the directory containing base
RMFEXPORT std::string get_relative_path(std::string base, std::string file);

// Get an absolute path to file from the directory containing base
RMFEXPORT std::string get_absolute_path(std::string base, std::string file);

RMFEXPORT std::string get_unique_path();
}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_PATHS_H */

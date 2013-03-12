/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_PATHS_H
#define RMF_INTERNAL_PATHS_H

#include <RMF/config.h>
#include <string>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {
RMFEXPORT std::string get_relative_path(std::string base,
                                        std::string file);
RMFEXPORT std::string get_absolute_path(std::string base,
                                        std::string file);
}   // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_PATHS_H */

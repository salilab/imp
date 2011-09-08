/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/errors.h>
#include <stdexcept>
namespace RMF {
  namespace internal {
    void handle_usage_error(std::string message) {
      throw std::runtime_error(message);
    }

    void handle_internal_error(std::string message) {
      throw std::logic_error(message);
    }

  } // namespace internal
} /* namespace RMF */

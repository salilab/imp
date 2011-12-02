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
    void handle_error(std::string) {
      //throw UsageException(message.c_str());
    }
  } // namespace internal
} /* namespace RMF */

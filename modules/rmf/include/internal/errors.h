/**
 *  \file IMP/rmf/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INTERNAL_ERRORS_H
#define IMPRMF_INTERNAL_ERRORS_H

#include "../rmf_config.h"

namespace RMF {
  namespace internal {

    IMPRMFEXPORT void handle_usage_error(std::string message);

    IMPRMFEXPORT void handle_internal_error(std::string message);

  } // namespace internal
} /* namespace RMF */

#endif /* IMPRMF_INTERNAL_ERRORS_H */

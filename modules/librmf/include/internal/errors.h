/**
 *  \file RMF/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_ERRORS_H
#define IMPLIBRMF_INTERNAL_ERRORS_H

#include "../RMF_config.h"

namespace RMF {
  namespace internal {

    RMFEXPORT void handle_usage_error(std::string message);

    RMFEXPORT void handle_internal_error(std::string message);

  } // namespace internal
} /* namespace RMF */

#endif /* IMPLIBRMF_INTERNAL_ERRORS_H */

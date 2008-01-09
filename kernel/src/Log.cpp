/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"

namespace IMP
{

/* Initialize singleton pointer to NULL */
Log* Log::logpt_ = NULL;


namespace internal {
  void assert_fail() {
    throw ErrorException();
  }

  void check_fail() {
  }
}
} // namespace IMP

/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/exception.h"

namespace IMP
{

/* Initialize singleton pointer to NULL */
Log* Log::logpt_ = NULL;

} // namespace IMP

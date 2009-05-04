/**
 *  \file directories.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/directories.h>
#include <IMP/internal/directories.h>
#include <cstdlib>

IMP_BEGIN_NAMESPACE

String get_data_directory()
{
  // Allow compiled-in value to be overridden by the user if necessary
  char *env = getenv("IMP_DATA_DIRECTORY");
  if (env) {
    return String(env);
  } else {
    // Default to compiled-in value
    return String(IMP_DATA_DIRECTORY);
  }
}

IMP_END_NAMESPACE

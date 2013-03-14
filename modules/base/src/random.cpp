/**
 *  \file random.cpp  \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/random.h"
#include <IMP/base/internal/static.h>

IMPBASE_BEGIN_NAMESPACE

boost::uint64_t get_random_seed() {
  return static_cast<boost::uint64_t >(internal::random_seed);
}

IMPBASE_END_NAMESPACE

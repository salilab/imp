/**
 *  \file random.cpp  \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/random.h"
#include <IMP/base/nullptr.h>
#include <ctime>

IMPBASE_BEGIN_NAMESPACE

::boost::rand48 random_number_generator(
                       static_cast<boost::uint64_t>(std::time(nullptr)));

IMPBASE_END_NAMESPACE

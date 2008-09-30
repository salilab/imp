/**
 *  \file random.cpp  \brief Random number generators used by IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/random.h"
#include <ctime>

IMP_BEGIN_NAMESPACE

::boost::rand48 random_number_generator(
                       static_cast<boost::uint64_t>(std::time(NULL)));

IMP_END_NAMESPACE

/**
 *  \file random.cpp  \brief Random number generators used by IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/random.h"
#include <cstdlib>

namespace IMP
{

::boost::rand48 random_number_generator(static_cast<int>(std::time(NULL)));  

} // namespace IMP

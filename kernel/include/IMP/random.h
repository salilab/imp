/**
 *  \file random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RANDOM_H
#define __IMP_RANDOM_H

#include <boost/random.hpp>
#include "base_types.h"

namespace IMP
{

typedef ::boost::rand48 RandomNumberGenerator;
extern IMPDLLEXPORT ::boost::rand48 random_number_generator;

} // namespace IMP

#endif  /* __IMP_RANDOM_H */

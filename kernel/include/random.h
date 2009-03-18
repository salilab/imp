/**
 *  \file random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_RANDOM_H
#define IMP_RANDOM_H

#include <boost/random.hpp>
#include "base_types.h"

IMP_BEGIN_NAMESPACE

typedef ::boost::rand48 RandomNumberGenerator;
//! A shared random number generator
/** The random number generator is seeded based of of the current time. To
    provide a better seed or use a constant seed call
    random_number_generator.seed(x) with a 32 bit int or a 64 bit unsigned int.
 */
extern IMPEXPORT RandomNumberGenerator random_number_generator;

IMP_END_NAMESPACE

#endif  /* IMP_RANDOM_H */

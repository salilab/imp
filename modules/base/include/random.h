/**
 *  \file IMP/base/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_RANDOM_H
#define IMPBASE_RANDOM_H

#include <IMP/base/base_config.h>
#include <boost/random.hpp>

IMPBASE_BEGIN_NAMESPACE

typedef ::boost::rand48 RandomNumberGenerator;
//! A shared random number generator
/** The random number generator is seeded based of of the current time. To
    provide a better seed or use a constant seed call
    random_number_generator.seed(x) with a 32 bit int or a 64 bit unsigned int.

    This generator can be used by the
\external{http://www.boost.org/doc/libs/1_39_0/libs/random/index.html, Boost.Random}
    distributions.
 */
extern IMPBASEEXPORT RandomNumberGenerator random_number_generator;


/** Return the initial random seed.
  */
IMPBASEEXPORT boost::uint64_t get_random_seed();

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_RANDOM_H */

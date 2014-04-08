/**
 *  \file IMP/base/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_RANDOM_H
#define IMPBASE_RANDOM_H

#include <IMP/base/base_config.h>
#include <boost/random/mersenne_twister.hpp>

IMPBASE_BEGIN_NAMESPACE
typedef ::boost::mt19937 RandomNumberGenerator;
//! A shared random number generator
/** The random number generator is seeded based using the boost::random_device
if it is available or `/dev/urandom` if it is not.

This generator can be used by the
[Boost.Random](http://www.boost.org/doc/libs/1_39_0/libs/random/index.html)
distributions.
 */
extern IMPBASEEXPORT RandomNumberGenerator random_number_generator;

/** Return the initial random seed.
  */
IMPBASEEXPORT boost::uint64_t get_random_seed();

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_RANDOM_H */

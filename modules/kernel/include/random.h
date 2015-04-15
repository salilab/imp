/**
 *  \file IMP/base/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include <IMP/base_config.h>
#include <boost/random/mersenne_twister.hpp>

IMPKERNEL_BEGIN_NAMESPACE
typedef ::boost::mt19937 RandomNumberGenerator;
//! A shared random number generator
/** The random number generator is seeded based using the boost::random_device
if it is available or `/dev/urandom` if it is not.

This generator can be used by the
[Boost.Random](http://www.boost.org/doc/libs/1_39_0/libs/random/index.html)
distributions.
 */
extern IMPKERNELEXPORT RandomNumberGenerator random_number_generator;

//! Return the initial random seed.
IMPKERNELEXPORT boost::uint64_t get_random_seed();

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_H */

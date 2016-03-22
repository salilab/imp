/**
 *  \file IMP/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <boost/random/mersenne_twister.hpp>

IMPKERNEL_BEGIN_NAMESPACE

#ifndef SWIG // the RNG is defined explicitly in pyext/IMP_kernel.random.i
typedef ::boost::mt19937 RandomNumberGenerator;

//! A shared non-GPU random number generator
/** The random number generator is seeded based on command line specified flag.
   Otherwise, the default seed is retrieved from among either
   boost::random_device, `/dev/urandom`, or the system clock, based on which
   method is available in this priority order.

This generator can be used by the
[Boost.Random](http://www.boost.org/doc/libs/1_39_0/libs/random/index.html)
distributions.
 */
extern IMPKERNELEXPORT RandomNumberGenerator random_number_generator;
#endif

//! Return the initial random seed.
IMPKERNELEXPORT boost::uint64_t get_random_seed();


IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_H */

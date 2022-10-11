/**
 *  \file IMP/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <boost/random/mersenne_twister.hpp>

IMPKERNEL_BEGIN_NAMESPACE

#ifndef SWIG // the RNG is defined explicitly in pyext/IMP_kernel.random.i

class RandomNumberGenerator : public ::boost::mt19937 {
  typedef ::boost::mt19937 T;
  T::result_type last_seed_;
  unsigned seed_counter_;

public:
  RandomNumberGenerator()
      : T(default_seed), last_seed_(default_seed), seed_counter_(1) {}

  RandomNumberGenerator(T::result_type seed)
      : T(seed), last_seed_(seed), seed_counter_(1) {}

  void seed() { seed(default_seed); }

  // Set a new seed
  void seed(T::result_type seed) {
    last_seed_ = seed;
    seed_counter_++;
    if (seed_counter_ == 0) {
      seed_counter_ = 1;
    }
    T::seed(seed);
  }

  // Get the last-set seed, either from when the class was constructed or
  // from a previous call to seed()
  T::result_type get_last_seed() const { return last_seed_; }

  // Get the counter value from when the seed was last set. This value
  // will never be zero. This can be used to determine if the seed was
  // changed since the last use.
  unsigned get_seed_counter() const { return seed_counter_; }
};

//! A shared non-GPU random number generator
/** The random number generator is seeded based on command line specified flag.
   Otherwise, the default seed is retrieved from among either
   boost::random_device, `/dev/urandom`, or the system clock, based on which
   method is available in this priority order.

   To set the seed, call the `seed` method, which takes a single integer
   argument. This object is also callable, and returns a new random integer
   on each call.

This generator can be used by the
[Boost.Random](http://www.boost.org/doc/libs/1_39_0/libs/random/index.html)
distributions.
 */
extern IMPKERNELEXPORT RandomNumberGenerator random_number_generator;
#endif

//! Return the initial random seed.
/** To set the seed or get random values, see IMP::random_number_generator.
 */
IMPKERNELEXPORT boost::uint64_t get_random_seed();


IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_H */

/**
 *  \file internal/IDGenerator.h
 *  \brief Generation of simple unique IDs.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_ID_GENERATOR_H
#define IMPKERNEL_INTERNAL_ID_GENERATOR_H

#include <IMP/kernel_config.h>
#include <random>
#include <cstdint>
#include <chrono>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Generator of simple IDs
/** These IDs are intended to be used to uniquely identify IMP Objects
    such as Model, to prevent users accidentally confusing multiple objects
    with the same name. These are generated with a simple random number
    generator - the chance of collision is very small (although not zero).
    The object pointer is not used as - depending on the dynamic memory
    implementation - a new object may get the address of a previously freed one.
 */
class IDGenerator {
  std::mt19937 rng_;
public:

  //! Constructor
  IDGenerator() {
    // Use system time to seed (don't use the regular IMP RNG as that may have
    // been seeded by the user)
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng_.seed(seed);
  }

  //! Generate and return a new ID
  uint32_t operator()() {
    // mt19937 returns a uint_fast32_t, which might be a larger type,
    // but we want exactly 32 bits
    return static_cast<uint32_t>(rng_());
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_ID_GENERATOR_H */

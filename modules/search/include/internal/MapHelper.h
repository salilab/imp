/**
 *  \file MapHelper.h   \brief A helper for the attribute map
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSEARCH_INTERNAL_MAP_HELPER_H
#define IMPSEARCH_INTERNAL_MAP_HELPER_H

#include "../config.h"
#include <IMP/base_types.h>

#include <IMP/Particle.h>

#include <boost/tuple/tuple.hpp>
#include <boost/static_assert.hpp>

#include <iostream>
#include <map>

IMPSEARCH_BEGIN_INTERNAL_NAMESPACE

template <class Value>
struct GetKey {
  typedef int Key;
};

template <>
struct GetKey<IMP::Float> {
  typedef FloatKey Key;
};

template <>
struct GetKey<IMP::Int> {
  typedef IntKey Key;
};

template <>
struct GetKey<IMP::Particle*> {
  typedef ParticleKey Key;
};

template <>
struct GetKey<String> {
  typedef StringKey Key;
};

IMPSEARCH_END_INTERNAL_NAMESPACE

#endif  /* IMPSEARCH_INTERNAL_MAP_HELPER_H */

/**
 *  \file compatibility/compatibility_config.h
 *  \brief Declare an efficient stl-compatible map
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMP_COMPATIBILITY_CONFIG_H
#define IMP_COMPATIBILITY_CONFIG_H

#include "../kernel_config.h"

#define IMP_BEGIN_COMPATIBILITY_NAMESPACE       \
  IMP_BEGIN_NAMESPACE                           \
  namespace compatibility {

#define IMP_END_COMPATIBILITY_NAMESPACE         \
  }                                             \
    IMP_END_NAMESPACE

/** \namespace IMP::compatibility

    \brief Headers and classes to make compatibility easier

    The functionality exposed in this module is there to smooth over differences
    in C++ environments on various platforms. It includes classes that select
    the best available implementation of some concept
    - IMP::compatibility::map
    - IMP::compatibility::set

    or headers that simply make sure the correct header is included

    - IMP/compatibility/vector_property_map.h
    - IMP/compatibility/hash.h
*/

#endif  /* IMP_COMPATIBILITY_CONFIG_H */

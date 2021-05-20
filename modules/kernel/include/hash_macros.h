/**
 *  \file IMP/hash_macros.h
 *  \brief Helper macros for implementing hashable classes.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_HASH_MACROS_H
#define IMPKERNEL_HASH_MACROS_H
#include <IMP/kernel_config.h>
#include <IMP/hash.h>

#ifdef IMP_DOXYGEN
/** Add the methods necessary to support insertion in
    Python dictionaries and map
    and set tables.*/
#define IMP_HASHABLE_INLINE(name, hashret)

#else

#define IMP_HASHABLE_INLINE(name, hashret) \
  std::size_t __hash__() const { hashret; }

#endif

#endif /* IMPKERNEL_HASH_MACROS_H */

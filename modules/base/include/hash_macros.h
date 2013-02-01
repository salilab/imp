/**
 *  \file IMP/base/hash_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_HASH_MACROS_H
#define IMPBASE_HASH_MACROS_H
#include <IMP/base/base_config.h>
#include <IMP/base/hash.h>

#ifdef IMP_DOXYGEN
/** Add the methods necessary to support insertion in
    python dictionaries and base::map
    and base::set tables.*/
#define IMP_HASHABLE_INLINE(name, hashret)

#else

#define IMP_HASHABLE_INLINE(name, hashret)\
  std::size_t __hash__() const {          \
    hashret;                              \
  }

#endif


#endif  /* IMPBASE_HASH_MACROS_H */

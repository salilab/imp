/**
 *  \file IMP/kernel/key_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_KEY_MACROS_H
#define IMPKERNEL_KEY_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/value_macros.h>
#include "Key.h"


/** Define a new key type.

    It defines two public types: Name, which is an instantiation of KeyBase, and
    Names which is a vector of Name.

    \param[in] Name The name for the new type.
    \param[in] Tag A (hopefully) unique integer to define this key type.

    \note We define a new class rather than use a typedef since SWIG has a
    bug dealing with names that start with ::. A fix has been commited to SVN
    for SWIG.

    \note The name in the typedef would have to start with ::IMP so it
    could be used out of the IMP namespace.
*/
#define IMP_DECLARE_KEY_TYPE(Name, Tag)         \
  typedef Key<Tag, true> Name;                  \
  IMP_VALUES(Name, Name##s)


/** Define a new key non lazy type where new types have to be created
    explicitly.

    \see IMP_DECLARE_KEY_TYPE
*/
#define IMP_DECLARE_CONTROLLED_KEY_TYPE(Name, Tag)      \
  typedef Key<Tag, false> Name;                         \
  IMP_VALUES(Name, Name##s)



#endif  /* IMPKERNEL_KEY_MACROS_H */

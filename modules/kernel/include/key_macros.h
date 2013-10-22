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
#include <IMP/base/deprecation_macros.h>
#include "Key.h"

/** \deprecated_at{2.1} Just declare the typedef directly.
*/
#define IMP_DECLARE_KEY_TYPE(Name, Tag)                                  \
  IMPKERNEL_DEPRECATED_MACRO(                                            \
      2.1, "Declare the typedef directly.") typedef Key<Tag, true> Name; \
  IMP_VALUES(Name, Name##s)

/** \deprecated_at{2.1} Just declare the typedef directly.
*/
#define IMP_DECLARE_CONTROLLED_KEY_TYPE(Name, Tag)                        \
  IMPKERNEL_DEPRECATED_MACRO(                                             \
      2.1, "Declare the typedef directly.") typedef Key<Tag, false> Name; \
  IMP_VALUES(Name, Name##s)

#endif /* IMPKERNEL_KEY_MACROS_H */

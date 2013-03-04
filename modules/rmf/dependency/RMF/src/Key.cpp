/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/Key.h>

RMF_COMPILER_ENABLE_WARNINGS

#  define RMF_DEFINE_KEY(lcname, Ucname, PassValue, ReturnValue, \
                          PassValues, ReturnValues)               \
  RMF_TEMPLATE_DEF(Key, Ucname##Traits, Ucname##Key)


RMF_FOREACH_TYPE(RMF_DEFINE_KEY);

RMF_COMPILER_DISABLE_WARNINGS

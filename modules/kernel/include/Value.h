/**
 *  \file IMP/Value.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_VALUE_H
#define IMPKERNEL_VALUE_H

#include <IMP/kernel_config.h>
#include "utility_macros.h"

IMPKERNEL_BEGIN_NAMESPACE
//! Base for a simple primitive-like type.
/** A value in \imp should support output to streams in C++, conversion to
    string in Python, being put in a hash table/dictionary and comparison
    with other values of the same type. In addition, its default constructor
    should put it into a known good initial state.
 */
class Value {
 protected:
  Value() {}
};
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_VALUE_H */

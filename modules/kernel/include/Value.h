/**
 *  \file IMP/Value.h
 *  \brief Base class for a simple primitive-like type.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_VALUE_H
#define IMPKERNEL_VALUE_H

#include <IMP/kernel_config.h>
#include "utility_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Base class for a simple primitive-like type.
/** A Value in \imp should support output to streams in C++, conversion to
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

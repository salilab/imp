/**
 *  \file IMP/base/Value.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_VALUE_H
#define IMPBASE_VALUE_H

#include <IMP/base/base_config.h>
#include "utility_macros.h"

IMPBASE_BEGIN_NAMESPACE
/** A value in \imp should support output to streams in C++, conversion to
    string in python, being put in a hash table/dictionary and comparison
    with other values of the same type. In addition, its default constructor
    should put it into a known good initial state.

*/
class Value {
 protected:
  Value(){}
};
IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_VALUE_H */

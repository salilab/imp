/**
 *  \file IMP/value_macros.h
 *  \brief Macros to help in implementing Value objects.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_VALUE_MACROS_H
#define IMPKERNEL_VALUE_MACROS_H
#include <IMP/kernel_config.h>
#include "Vector.h"

//! Define the type for storing sets of values
/** The macro defines the type Names. PluralName should be
    Names unless the English spelling is
    different. This macro also defines the output operator
    for the type.

    See \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.
 */
#define IMP_VALUES(Name, PluralName) \
  /** Pass or store a set of Name. */ typedef IMP::Vector<Name> PluralName

/** To be used with native types.*/
#define IMP_BUILTIN_VALUES(Name, PluralName) \
  IMP_VALUES(Name, PluralName);              \
  IMP_VALUES(PluralName, PluralName##s)

#endif /* IMPKERNEL_VALUE_MACROS_H */

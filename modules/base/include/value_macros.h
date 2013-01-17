/**
 *  \file IMP/base/value_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_VALUE_MACROS_H
#define IMPBASE_VALUE_MACROS_H
#include <IMP/base/base_config.h>
#include "Vector.h"

//! Define the type for storing sets of values
/** The macro defines the type Names. PluralName should be
    Names unless the English spelling is
    different. This macro also defines the output operator
    for the type.

    See
    \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.
 */
#define IMP_VALUES(Name, PluralName)            \
  /** Pass or store a set of Name. */           \
typedef IMP::base::Vector<Name> PluralName


/** To be used with native types.*/
#define IMP_BUILTIN_VALUES(Name, PluralName)                     \
    IMP_VALUES(Name, PluralName);                                \
    IMP_VALUES(PluralName, PluralName##s)



#endif  /* IMPBASE_VALUE_MACROS_H */

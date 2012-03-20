/**
 *  \file IMP/base/value_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_VALUE_MACROS_H
#define IMPBASE_VALUE_MACROS_H
#include "base_config.h"
#include "Vector.h"

#ifdef IMP_DOXYGEN
//! Define the type for storing sets of values
/** The macro defines the type Names. PluralName should be
    Names unless the English spelling is
    different. This macro also defines the output operator
    for the type.

    See
    \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.
 */
#define IMP_VALUES(Name, PluralName)
#else
#define IMP_VALUES(Name, PluralName)                            \
  typedef IMP::base::Vector<Name> PluralName
#endif


/*   inline std::ostream &operator<<(std::ostream &out,            \
                                  const Name &n) {              \
    return out << static_cast<IMP::base::Showable>(n);          \
  }                                                             \
*/

#ifdef IMP_DOXYGEN
/** This is like IMP_VALUES() but for built in types that have
    now show.
 */
#define IMP_BUILTIN_VALUES(Name, PluralName)
#else
#define IMP_BUILTIN_VALUES(Name, PluralName)                     \
  typedef IMP::base::Vector<Name> PluralName;                    \
  typedef IMP::base::Vector<PluralName> PluralName##s
#endif



#endif  /* IMPBASE_VALUE_MACROS_H */

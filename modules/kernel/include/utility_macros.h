/**
 *  \file IMP/utility_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UTILITY_MACROS_H
#define IMPKERNEL_UTILITY_MACROS_H
#include <IMP/kernel_config.h>
#include <IMP/deprecation_macros.h>

//! Use a copy_from method to create a copy constructor and operator=
/** This macro is there to aid with classes which require a custom
    copy constructor. It simply forwards \c operator= and the copy
    constructor to a method \c copy_from() which should do the copying.

    You should think very hard before implementing a class which
    requires a custom copy constructor as it is easy to get wrong
    and you can easily wrap most resources with RAII objects
    (\external{http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization,
    Wikipedia entry}).
*/
#define IMP_COPY_CONSTRUCTOR(Name, Base)         \
  Name(const Name& o) : Base() { copy_from(o); } \
  IMP_NO_SWIG(Name& operator=(const Name& o) {   \
    copy_from(o);                                \
    return *this;                                \
  }) IMP_REQUIRE_SEMICOLON_CLASS(copy)

#define IMP_EXPAND_AND_STRINGIFY(x) IMP_STRINGIFY(x)

#endif /* IMPKERNEL_UTILITY_MACROS_H */

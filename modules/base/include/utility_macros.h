/**
 *  \file IMP/base/utility_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_UTILITY_MACROS_H
#define IMPBASE_UTILITY_MACROS_H
#include <IMP/base/base_config.h>
#include <IMP/base/deprecation_macros.h>

//! Use a copy_from method to create a copy constructor and operator=
/** This macro is there to aid with classes which require a custom
    copy constructor. It simply forwards \c operator= and the copy
    constructor to a method \c copy_from() which should do the copying.

    You should think very hard before implementing a class which
    requires a custom copy custructor as it is easy to get wrong
    and you can easily wrap most resources with RAII objects
    (\external{http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization,
    wikipedia entry}).
*/
#define IMP_COPY_CONSTRUCTOR(Name, Base)         \
  Name(const Name& o) : Base() { copy_from(o); } \
  IMP_NO_SWIG(Name& operator=(const Name& o) {   \
    copy_from(o);                                \
    return *this;                                \
  }) IMP_REQUIRE_SEMICOLON_CLASS(copy)

#define IMP_EXPAND_AND_STRINGIFY(x) IMP_STRINGIFY(x)

#endif /* IMPBASE_UTILITY_MACROS_H */

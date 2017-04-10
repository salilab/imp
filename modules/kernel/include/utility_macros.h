/**
 *  \file IMP/utility_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
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

#ifdef IMP_DOXYGEN
//! Smart pointer to retain sole ownership of an object through a pointer
/** In C++11 mode, this is std::unique_ptr; otherwise, it is std::auto_ptr.
    Note that these two classes do not have exactly the same interfaces, so
    you must be careful to use IMP_UNIQUE_PTR only in cases where the two
    classes behave in the same way (e.g. you cannot copy a unique_ptr, but
    you can copy an auto_ptr; unique_ptr works with arrays and can
    be stored in STL containers, unlike auto_ptr).
 */
#define IMP_UNIQUE_PTR
#else
#if IMP_COMPILER_HAS_UNIQUE_PTR
#define IMP_UNIQUE_PTR std::unique_ptr
#else
#define IMP_UNIQUE_PTR std::auto_ptr
#endif
#endif

#endif /* IMPKERNEL_UTILITY_MACROS_H */

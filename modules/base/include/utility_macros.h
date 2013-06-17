/**
 *  \file IMP/base/utility_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
  Name(const Name &o) : Base() { copy_from(o); } \
  IMP_NO_SWIG(Name &operator=(const Name &o) {   \
    copy_from(o);                                \
    return *this;                                \
  }) IMP_REQUIRE_SEMICOLON_CLASS(copy)

/** \deprecated_at{2.1} Don't use this */
#define IMP_PROTECTED_METHOD_DECL(protection, return_value, name, arguments, \
                                  const_or_not, body)                        \
  IMPBASE_DEPRECATED_MACRO(2.1, "Don't use this.");                          \
  protection:                                                                \
  return_value name arguments const_or_not body

/** \deprecated_at{2.1} Don't use this */
#define IMP_PROTECTED_CONSTRUCTOR_DECL(protection, Name, arguments, body) \
  protection:                                                             \
  Name arguments body

/** \deprecated_at{2.1} Don't use this */
#define IMP_PROTECTED_DESTRUCTOR_DECL(protection, Name, arguments, body) \
  protection:                                                            \
  virtual ~Name arguments body

#if !defined(SWIG)
/** \deprecated_at{2.1} Don't use this */
#define IMP_PROTECTED_METHOD(return_value, name, arguments, const_or_not, \
                             body)                                        \
  IMPBASE_DEPRECATED_MACRO(2.1, "Don't use this");                        \
  IMP_PROTECTED_METHOD_DECL(protected, return_value, name, arguments,     \
                            const_or_not, body)

/** \deprecated_at{2.1} Don't use this */
#define IMP_PROTECTED_CONSTRUCTOR(Name, arguments, body) \
  IMPBASE_DEPRECATED_MACRO(2.1, "Don't use this");       \
  IMP_PROTECTED_CONSTRUCTOR_DECL(protected, Name, arguments, body)

/** \deprecated_at{2.1} Don't use this */
#define IMP_PROTECTED_DESTRUCTOR(Name, arguments, body) \
  IMPBASE_DEPRECATED_MACRO(2.1, "Don't use this");      \
  IMP_PROTECTED_DESTRUCTOR_DECL(protected, Name, arguments, body)

#ifndef IMP_DOXYGEN
/** \deprecated_at{2.1} Don't use this */
#define IMP_INTERNAL_METHOD(return_value, name, arguments, const_or_not, body) \
  IMPBASE_DEPRECATED_MACRO(2.1, "Don't use this");                             \
                                                                               \
 public:                                                                       \
  return_value name arguments const_or_not body

#else
/** \deprecated_at{2.1} Don't use this */
#define IMP_INTERNAL_METHOD(return_value, name, arguments, const_or_not, body)
#endif

#else
#define IMP_PROTECTED_METHOD(return_value, name, arguments, const_or_not, \
                             body)                                        \
  % rename(_##name) name;                                                 \
  IMP_PROTECTED_METHOD_DECL(public, return_value, name, arguments,        \
                            const_or_not, body)

#define IMP_INTERNAL_METHOD(return_value, name, arguments, const_or_not, body)

#define IMP_PROTECTED_CONSTRUCTOR(Name, arguments, body) \
  IMP_PROTECTED_CONSTRUCTOR_DECL(public, Name, arguments, body)

#define IMP_PROTECTED_DESTRUCTOR(Name, arguments, body) \
  IMP_PROTECTED_DESTRUCTOR_DECL(public, Name, arguments, body)

#endif

#define IMP_EXPAND_AND_STRINGIFY(x) IMP_STRINGIFY(x)

#endif /* IMPBASE_UTILITY_MACROS_H */

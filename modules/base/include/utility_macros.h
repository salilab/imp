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
#define IMP_COPY_CONSTRUCTOR(Name, Base) Name(const Name &o): Base()    \
  {copy_from(o);}                                                       \
  IMP_NO_SWIG(Name& operator=(const Name &o) {copy_from(o); return *this;}) \
  IMP_REQUIRE_SEMICOLON_CLASS(copy)

#define IMP_PROTECTED_METHOD_DECL(protection, return_value, name, arguments, \
                                  const_or_not, body)                   \
  protection:                                                           \
  return_value name arguments const_or_not body

#define IMP_PROTECTED_CONSTRUCTOR_DECL(protection, Name, arguments, body) \
  protection:                                                           \
  Name arguments body

#define IMP_PROTECTED_DESTRUCTOR_DECL(protection, Name, arguments, body) \
  protection:                                                           \
  virtual ~Name arguments body

#if defined(IMP_DOXYGEN)
//! Use this when declaring methods protected
/** Using this macro instead of just \c protected: ensures that protected
    methods are available for use in Python as well as in C++. The methods
    are made availabe in python with an \c _ prefix to denote that they
    are protected.*/
#define IMP_PROTECTED_METHOD(return_value, name, arguments, const_or_not, \
                             body)                                      \
  IMP_PROTECTED_METHOD_DECL(protected, return_value, name, arguments,\
                            const_or_not, body)


//! Use this when declaring constructors protected
/** Using this macro instead of just \c protected: ensures that protected
    constructors are available for use in Python as well as in C++.*/
#define IMP_PROTECTED_CONSTRUCTOR(Name, arguments, body)                \
  IMP_PROTECTED_CONSTRUCTOR_DECL(protected, Name, arguments, body)

//! Use this when declaring destructors protected
/** Using this macro instead of just \c protected: ensures that protected
    constructors are available for use in Python as well as in C++.
    \note These destructors are virtual destructors. Most classes that use
    this have virtual methods.
*/
#define IMP_PROTECTED_DESTRUCTOR(Name, arguments, body)                 \
  IMP_PROTECTED_DESTRUCTOR_DECL(protected, Name, arguments, body)


/** Use this to declare a method that should be hidden from swig and
    the docs, but can't be private so some reason or another. Not
    something you should do often.*/
#define IMP_INTERNAL_METHOD(return_value, name, arguments, const_or_not, \
                            body)

#elif defined(IMP_SWIG_WRAPPER) || defined(_MSC_VER)
#define IMP_PROTECTED_METHOD(return_value, name, arguments,             \
                             const_or_not, body)                        \
  IMP_PROTECTED_METHOD_DECL(public, return_value, name, arguments,      \
                       const_or_not, body)

#define IMP_INTERNAL_METHOD(return_value, name, arguments,              \
                            const_or_not, body)                         \
  IMP_PROTECTED_METHOD_DECL(public, return_value, name, arguments,      \
                            const_or_not, body)



#define IMP_PROTECTED_CONSTRUCTOR(Name, arguments, body)        \
  IMP_PROTECTED_CONSTRUCTOR_DECL(public, Name, arguments, body)

#define IMP_PROTECTED_DESTRUCTOR(Name, arguments, body)         \
  IMP_PROTECTED_DESTRUCTOR_DECL(public, Name, arguments, body)

#elif defined(SWIG)
#define IMP_PROTECTED_METHOD(return_value, name, arguments,             \
                             const_or_not, body)                        \
  %rename(_##name) name;                                                \
  IMP_PROTECTED_METHOD_DECL(public, return_value, name, arguments,      \
                       const_or_not, body)

#define IMP_INTERNAL_METHOD(return_value, name, arguments,      \
                             const_or_not, body)


#define IMP_PROTECTED_CONSTRUCTOR(Name, arguments, body)        \
  IMP_PROTECTED_CONSTRUCTOR_DECL(public, Name, arguments, body)

#define IMP_PROTECTED_DESTRUCTOR(Name, arguments, body)         \
  IMP_PROTECTED_DESTRUCTOR_DECL(public, Name, arguments, body)


#else
#define IMP_PROTECTED_METHOD(return_value, name, arguments, const_or_not, \
                             body)                                      \
  IMP_PROTECTED_METHOD_DECL(protected, return_value, name, arguments,   \
                       const_or_not, body)

#define IMP_INTERNAL_METHOD(return_value, name, arguments, const_or_not, \
                            body)                                       \
  IMP_PROTECTED_METHOD_DECL(public, return_value, name, arguments,      \
                       const_or_not, body)


#define IMP_PROTECTED_CONSTRUCTOR(Name, arguments, body)                \
  IMP_PROTECTED_CONSTRUCTOR_DECL(protected, Name, arguments, body)


#define IMP_PROTECTED_DESTRUCTOR(Name, arguments, body)                 \
  IMP_PROTECTED_DESTRUCTOR_DECL(protected, Name, arguments, body)

#endif

#define IMP_STRINGIFY(x) #x

#define IMP_EXPAND_AND_STRINGIFY(x) IMP_STRINGIFY(x)


#endif  /* IMPBASE_UTILITY_MACROS_H */

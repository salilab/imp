/**
 *  \file IMP/base/swap_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_SWAP_MACROS_H
#define IMPBASE_SWAP_MACROS_H
#include <IMP/base/base_config.h>

#if !defined(SWIG)
/** \name Swap helpers

    Use the swap_with member function to swap two objects. The two
    objects must be of the same type (Name) and define
    the method \c swap_with(). The number suffix is the number of template
    arguments, all of which must be of class type.
    @{
*/
#define IMP_SWAP(Name)                                   \
  inline void swap(Name& a, Name& b) { a.swap_with(b); } \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_SWAP_1(Name)                     \
  template <class A>                         \
  inline void swap(Name<A>& a, Name<A>& b) { \
    a.swap_with(b);                          \
  }                                          \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_SWAP_2(Name)                           \
  template <class A, class B>                      \
  inline void swap(Name<A, B>& a, Name<A, B>& b) { \
    a.swap_with(b);                                \
  }                                                \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_SWAP_3(Name)                                 \
  template <class A, class B, class C>                   \
  inline void swap(Name<A, B, C>& a, Name<A, B, C>& b) { \
    a.swap_with(b);                                      \
  }                                                      \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

/** @} */
#else
#define IMP_SWAP(Name)

#define IMP_SWAP_1(Name)

#define IMP_SWAP_2(Name)

#define IMP_SWAP_3(Name)
#endif

//! Swap two member variables assuming the other object is called o
/** Swap the member \c var_name of the two objects (this and o).
 */
#define IMP_SWAP_MEMBER(var_name) swap(var_name, o.var_name)

#endif /* IMPBASE_SWAP_MACROS_H */

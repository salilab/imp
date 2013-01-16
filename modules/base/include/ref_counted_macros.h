/**
 *  \file IMP/base/ref_counted_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_REF_COUNTED_MACROS_H
#define IMPBASE_REF_COUNTED_MACROS_H
#include <IMP/base/base_config.h>
#include "utility_macros.h"


#ifdef _MSC_VER
// VC doesn't understand friends properly
#define IMP_REF_COUNTED_DESTRUCTOR(Name)        \
  public:                                       \
  virtual ~Name(){}                             \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)              \
  public:                                                          \
  virtual ~Name(){dest}                                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
  virtual ~Name()


#elif defined(SWIG)
// SWIG doesn't do friends right either, but we don't care as much
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                \
  public:                                               \
  virtual ~Name(){}                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)              \
  public:                                                          \
  virtual ~Name(){dest}                                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
  virtual ~Name()


#elif defined(IMP_DOXYGEN)
/* The destructor is unprotected for SWIG since if it is protected
   SWIG does not wrap the Python proxy distruction and so does not
   dereference the ref counted pointer. SWIG also gets confused
   on template friends.
*/
//! Ref counted objects should have private destructors
/** This macro defines a private destructor and adds the appropriate
    friend methods so that the class can be used with ref counting.
    By defining a private destructor, you make it so that the object
    cannot be declared on the stack and so must be ref counted.

    \see IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR()
    \see IMP::base::RefCounted
*/
#define IMP_REF_COUNTED_DESTRUCTOR(Name)


/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is only
    declared, not defined.
*/
#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)

/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is only
    declared, not defined and is virtual.
*/
#define IMP_REF_COUNTED_NONTRIVIAL_VIRTUAL_DESTRUCTOR(Name)


/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is declared
    inline.
*/
#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, destructor)


#else
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                                \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  ~Name(){}                               \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)                   \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  ~Name(){dest}                           \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)                     \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  ~Name();                                  \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_NONTRIVIAL_VIRTUAL_DESTRUCTOR(Name)             \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  virtual ~Name();                                                      \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)
#endif

#endif  /* IMPBASE_REF_COUNTED_MACROS_H */

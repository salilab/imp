/**
 *  \file IMP/ref_counted_macros.h
 *  \brief Macros to help with reference counting.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_REF_COUNTED_MACROS_H
#define IMPKERNEL_REF_COUNTED_MACROS_H
#include <IMP/kernel_config.h>
#include "utility_macros.h"

#if defined(IMP_DOXYGEN)
//! Set up destructor for a ref counted object
/** This macro defines a virtual destructor for a ref counted object.
    Ideally, the destructor would be defined private, so that the object
    cannot be declared on the stack and so must be ref counted, but in
    practice this breaks usage of the object with SWIG, some older compilers,
    C++11 smart pointers (e.g. std::unique_ptr, std::shared_ptr), and
    serialization, so a public destructor is used.

    \see IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR()
*/
#define IMP_REF_COUNTED_DESTRUCTOR(Name)

/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is only
    declared, not defined.
*/
#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)

/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is declared
    inline.
*/
#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, destructor)

#else
#define IMP_REF_COUNTED_DESTRUCTOR(Name) \
 public:                                 \
  virtual ~Name() {}                     \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest) \
 public:                                              \
  virtual ~Name() { dest }                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name) \
 public:                                            \
  virtual ~Name();                                  \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)
#endif

#endif /* IMPKERNEL_REF_COUNTED_MACROS_H */

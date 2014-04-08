/**
 *  \file IMP/base/doxygen_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_DOXYGEN_MACROS_H
#define IMPBASE_DOXYGEN_MACROS_H
#include <IMP/base/base_config.h>
// for IMP_OVERRIDE
#include "swig_macros.h"

#ifdef IMP_DOXYGEN
//! Hide something from doxygen
/** */
#define IMP_NO_DOXYGEN(x)
//! Only show something to doxygen
/** */
#define IMP_ONLY_DOXYGEN(x) x IMP_REQUIRE_SEMICOLON_CLASS(only_doxy)
#define IMP_SWITCH_DOXYGEN(x, y) x
#else
#define IMP_NO_DOXYGEN(x) x
#define IMP_ONLY_DOXYGEN(x) IMP_REQUIRE_SEMICOLON_CLASS(only_doxy)
#define IMP_SWITCH_DOXYGEN(x, y) y
#endif

#ifdef IMP_DOXYGEN
/** Declare a method that implements a method that is pure virtual in the
    base class.
*/
#define IMP_IMPLEMENT(signature)
/** Define a method inline that impements a pure virtual method.
 */
#define IMP_IMPLEMENT_INLINE(signature, body)

/** Define a method inline that impements a pure virtual method that
    is hidden from swig.
 */
#define IMP_IMPLEMENT_INLINE_NO_SWIG(signature, body)

/** Define an implementation detail template method.
 */
#define IMP_IMPLEMENTATION_TEMPLATE_1(arg0, signature, body)
#define IMP_IMPLEMENTATION_TEMPLATE_2(arg0, arg1, signature, body)
#else
#define IMP_IMPLEMENT(signature) signature

#define IMP_IMPLEMENT_INLINE(signature, body) \
  signature IMP_OVERRIDE { body; }

#ifndef SWIG
#define IMP_IMPLEMENT_INLINE_NO_SWIG(signature, body) \
  signature IMP_OVERRIDE { body; }
#else
#define IMP_IMPLEMENT_INLINE_NO_SWIG(signature, body)

#endif

#define IMP_IMPLEMENTATION_TEMPLATE_1(arg0, signature, body) \
  template <arg0>                                            \
  signature IMP_OVERRIDE {                                   \
    body                                                     \
  }

#define IMP_IMPLEMENTATION_TEMPLATE_2(arg0, arg1, signature, body) \
  template <arg0, arg1>                                            \
  signature IMP_OVERRIDE {                                         \
    body                                                           \
  }

#endif

#endif /* IMPBASE_DOXYGEN_MACROS_H */

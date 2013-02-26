/**
 *  \file IMP/base/showable_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_SHOWABLE_MACROS_H
#define IMPBASE_SHOWABLE_MACROS_H
#include <IMP/base/base_config.h>
#include "Showable.h"
#include "warning_macros.h"

#if defined(IMP_DOXYGEN) || defined(SWIG)
/** \name Showable
    Declare the methods needed by an object that can be printed,
    both from C++ and Python. Each value-type class should have an
    IMP_SHOWABLE() call internal to it and an IMP_OUTPUT_OPERATOR()
    call external to it.

    The suffixs are the number of template arguments that the
    object has (eg _1 means one template argument). _D means
    one integer template argument.
    @{
*/

/** This macro declares the method
    - void show(std::ostream &out) const
    It also makes it so that the object can be printed
    in Python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE().

    Do not use with IMP::Object objects as they have their
    own show mechanism.
*/
#define IMP_SHOWABLE(Name)                              \
  void show(std::ostream &out=std::cout) const

//! Declare the methods needed by an object that can be printed
/** This macro declares the method
    - \c void \c show(std::ostream &out) const
    It also makes it so that the object can be printed
    in Python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE()
*/
#define IMP_SHOWABLE_INLINE(Name, how_to_show)  \
  void show(std::ostream &out=std::cout) const
/** @} */

#else

#define IMP_SHOWABLE(Name)                                              \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  void show(std::ostream &out=std::cout) const;                         \
  operator IMP::base::Showable() const {                                \
    std::ostringstream oss;                                             \
    show(oss);                                                          \
    return IMP::base::Showable(oss.str());                              \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS                                         \
  IMP_REQUIRE_SEMICOLON_CLASS(showable)

#define IMP_SHOWABLE_INLINE(Name, how_to_show)                          \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  void show(std::ostream &out=std::cout) const{                         \
    how_to_show;                                                        \
  }                                                                     \
  operator IMP::base::Showable() const {                                \
    std::ostringstream oss;                                             \
    show(oss);                                                          \
    return IMP::base::Showable(oss.str());                              \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS                                        \
  IMP_REQUIRE_SEMICOLON_CLASS(showable)

#endif

/** @} */


#endif  /* IMPBASE_SHOWABLE_MACROS_H */

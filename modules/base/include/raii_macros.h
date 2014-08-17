/**
 *  \file IMP/base/raii_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_RAII_MACROS_H
#define IMPBASE_RAII_MACROS_H
#include <IMP/base/base_config.h>
#include "showable_macros.h"
#include "RAII.h"

//! Declares RAII-style methods in a class
/** Since such class methods are typically quite small and simple, all
    the implementation is inline. The macro declares
    - default constructor
    - explicit constructor
    - RAII::set()
    - RAII::reset()
    - destructor

    @param Name the class name
    @param args the argument string (in parens) for the explicit constructor
                and set()
    @param Initialize code called from any constructor, including the default
    @param Set the code called from the explicit constructor or the set()
               function
    @param Reset the code called from the destructor, and in set before calling
                 the Set code
    @param Show the code for the show() method
*/
#define IMP_RAII(Name, args, Initialize, Set, Reset, Show)                     \
  IMP_HELPER_MACRO_PUSH_WARNINGS Name() { Initialize; }                        \
  /** explicit constructor that sets the properties of Name */                 \
  explicit Name args {                                                         \
    Initialize;                                                                \
    Set;                                                                       \
  }                                                                            \
  /** sets the properties of the class to new ones */                          \
  void set args {                                                              \
    reset();                                                                   \
    Set;                                                                       \
  }                                                                            \
  /** resets the properties previously set for this class */                   \
  void reset() { Reset; }                                                      \
  ~Name() { reset(); }                                                         \
  IMP_HELPER_MACRO_POP_WARNINGS IMP_SHOWABLE_INLINE(Name, out << #Name << '('; \
                                                    Show; out << ')')

#endif /* IMPBASE_RAII_MACROS_H */

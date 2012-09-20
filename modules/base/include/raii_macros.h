/**
 *  \file IMP/base/raii_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_RAII_MACROS_H
#define IMPBASE_RAII_MACROS_H
#include "base_config.h"
#include "showable_macros.h"
#include "RAII.h"

//! Declare a RAII-style class
/** Since such classes are typically quite small and simple, all
    the implementation is inline. The macro declares
    - default constructor
    - RAII::set()
    - RAII::reset()
    - destructor
    The Set and Reset arguments are the code to run in the respective
    functions. "args" are the argument string (in parens) for the
    constructor and set.
*/
#define IMP_RAII(Name, args, Initialize, Set, Reset, Show)              \
  Name() {Initialize;}                                                  \
  explicit Name args {Initialize; Set;}                                 \
  void set args {reset();                                               \
    Set;}                                                               \
  void reset() {Reset;}                                                 \
  ~Name () {reset();}                                                   \
  IMP_SHOWABLE_INLINE(Name, out << #Name << '('; Show; out << ')')




#endif  /* IMPBASE_RAII_MACROS_H */

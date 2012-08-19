/**
 *  \file IMP/base/tuple_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_TUPLE_MACROS_H
#define IMPBASE_TUPLE_MACROS_H
#include "base_config.h"
#include "Value.h"
#include "Showable.h"
#include "hash.h"

/** \name Named tuples
    It is often useful to declare little structures to aid in the passing
    of arguments by name or returning sets of values. One can use
    boost::tuples, but these don't have names for their parts and so
    don't lead to clear code. Instead we provide a macro to aid
    declaring such classes. The resulting class is hashable and
    comparable too.
@{
*/

#define IMP_NAMED_TUPLE_1(Name, Names, type0, var0, invariant)          \
  struct Name: public IMP::base::Value {                                \
    type0 var0;                                                         \
    Name(type0 i0=type0()): var0(i0){invariant;}                        \
    IMP_HASHABLE_INLINE(Name, {                                         \
        using IMP::base::hash_value;                                    \
        std::size_t value= hash_value(var0);                            \
        return value;                                                   \
      });                                                               \
    IMP_COMPARISONS_1(Name, var0);                                      \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0) << ")");           \
  };                                                                    \
  IMP_VALUES(Name, Names)



#define IMP_NAMED_TUPLE_2(Name, Names, type0, var0, type1, var1,        \
                          invariant)                                    \
  struct Name: public IMP::base::Value {                                \
    type0 var0;                                                         \
    type1 var1;                                                         \
    Name(type0 i0=type0(), type1 i1=type1()): var0(i0), var1(i1)        \
    {invariant;}                                                        \
    IMP_HASHABLE_INLINE(Name, {                                         \
        using IMP::base::hash_value;                                    \
        std::size_t value= hash_value(var0);                            \
        boost::hash_combine(value, hash_value(var1));                   \
        return value;                                                   \
      });                                                               \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0)                    \
                        << " " <<#var1 << "="                           \
                        << IMP::base::Showable(var1) << ")");           \
    IMP_COMPARISONS_2(Name, var0, var1);                                \
  };                                                                    \
  IMP_VALUES(Name, Names)


#define IMP_NAMED_TUPLE_3(Name, Names, type0, var0, type1, var1,        \
                          type2, var2, invariant)                       \
  struct Name: public IMP::base::Value {                                \
    type0 var0;                                                         \
    type1 var1;                                                         \
    type2 var2;                                                         \
    Name(type0 i0=type0(), type1 i1=type1(),type2 i2=type2()            \
         ): var0(i0), var1(i1), var2(i2){invariant;}                    \
    IMP_HASHABLE_INLINE(Name, {                                         \
        using IMP::base::hash_value;                                    \
        std::size_t value= hash_value(var0);                            \
        boost::hash_combine(value, hash_value(var1));                   \
        boost::hash_combine(value, hash_value(var2));                   \
        return value;                                                   \
      });                                                               \
    IMP_COMPARISONS_3(Name, var0, var1, var2);                          \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0)                    \
                        << " " <<#var1 << "="                           \
                        << IMP::base::Showable(var1)                    \
                        << " " <<#var2 << "="                           \
                        << IMP::base::Showable(var2) << ")");           \
  };                                                                    \
  IMP_VALUES(Name, Names)

/**@}*/

#endif  /* IMPBASE_TUPLE_MACROS_H */

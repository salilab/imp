/**
 *  \file IMP/base/tuple_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_TUPLE_MACROS_H
#define IMPBASE_TUPLE_MACROS_H
#include <IMP/base/base_config.h>
#include "Value.h"
#include "Showable.h"
#include "hash.h"
#include "hash_macros.h"
#include "showable_macros.h"
#include "value_macros.h"
#include "comparison_macros.h"
#include "swig_macros.h"

/** Implementation macro */
#define IMP_TUPLE_VALUE(Name, type_name, data_name, var_name)   \
  private:                                                      \
  type_name var_name;                                           \
public:                                                         \
 IMP_HELPER_MACRO_PUSH_WARNINGS                                 \
 const type_name &get_##data_name() const {return var_name;}    \
 void set_##data_name(const type_name &v) {                     \
   var_name=v;                                                  \
 }                                                              \
 IMP_NO_SWIG(type_name& access_##data_name() {return var_name;})\
 IMP_HELPER_MACRO_POP_WARNINGS


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
    Name(type0 i0=type0()): var0##_(i0){invariant;}                     \
    IMP_HASHABLE_INLINE(Name, {                                         \
        std::size_t value= IMP::base::hash_value(var0##_);              \
        return value;                                                   \
      });                                                               \
    IMP_COMPARISONS_1(Name);                                            \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0##_) << ")");        \
    IMP_TUPLE_VALUE(Name, type0, var0, var0##_);                        \
  private:                                                              \
  int compare(const Name &o) const {                                    \
    IMP_COMPARE_ONE(var0##_, o.var0##_);                                \
    return 0;                                                           \
  }                                                                     \
  };                                                                    \
  IMP_VALUES(Name, Names)



#define IMP_NAMED_TUPLE_2(Name, Names, type0, var0, type1, var1,        \
                          invariant)                                    \
  struct Name: public IMP::base::Value {                                \
    Name(type0 i0=type0(), type1 i1=type1()): var0##_(i0), var1##_(i1)  \
    {invariant;}                                                        \
    IMP_HASHABLE_INLINE(Name, {                                         \
        std::size_t value= IMP::base::hash_value(var0##_);              \
        boost::hash_combine(value, IMP::base::hash_value(var1##_));     \
        return value;                                                   \
      });                                                               \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0##_)                 \
                        << " " <<#var1 << "="                           \
                        << IMP::base::Showable(var1##_) << ")");        \
    IMP_COMPARISONS(Name);                                              \
    IMP_TUPLE_VALUE(Name, type0, var0, var0##_);                        \
    IMP_TUPLE_VALUE(Name, type1, var1, var1##_);                        \
  private:                                                              \
  int compare(const Name &o) const {                                    \
    IMP_COMPARE_ONE(var0##_, o.var0##_);                                \
    IMP_COMPARE_ONE(var1##_, o.var1##_);                                \
    return 0;                                                           \
  }                                                                     \
  };                                                                    \
  IMP_VALUES(Name, Names)


#define IMP_NAMED_TUPLE_3(Name, Names, type0, var0, type1, var1,        \
                          type2, var2, invariant)                       \
  struct Name: public IMP::base::Value {                                \
    Name(type0 i0=type0(), type1 i1=type1(),type2 i2=type2()            \
         ): var0##_(i0), var1##_(i1), var2##_(i2){invariant;}           \
    IMP_HASHABLE_INLINE(Name, {                                         \
        std::size_t value= IMP::base::hash_value(var0##_);              \
        boost::hash_combine(value, IMP::base::hash_value(var1##_));     \
        boost::hash_combine(value, IMP::base::hash_value(var2##_));     \
        return value;                                                   \
      });                                                               \
    IMP_COMPARISONS(Name);                                              \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0##_)                 \
                        << " " <<#var1 << "="                           \
                        << IMP::base::Showable(var1##_)                 \
                        << " " <<#var2 << "="                           \
                        << IMP::base::Showable(var2##_) << ")");        \
    IMP_TUPLE_VALUE(Name, type0, var0, var0##_);                        \
    IMP_TUPLE_VALUE(Name, type1, var1, var1##_);                        \
    IMP_TUPLE_VALUE(Name, type2, var2, var2##_);                        \
  private:                                                              \
  int compare(const Name &o) const {                                    \
    IMP_COMPARE_ONE(var0##_, o.var0##_);                                \
    IMP_COMPARE_ONE(var1##_, o.var1##_);                                \
    IMP_COMPARE_ONE(var2##_, o.var2##_);                                \
    return 0;                                                           \
  }                                                                     \
  };                                                                    \
  IMP_VALUES(Name, Names)


#define IMP_NAMED_TUPLE_4(Name, Names, type0, var0, type1, var1,        \
                          type2, var2, type3, var3, invariant)          \
  struct Name: public IMP::base::Value {                                \
    Name(type0 i0=type0(), type1 i1=type1(),type2 i2=type2(),           \
         type3 i3=type3()): var0##_(i0), var1##_(i1), var2##_(i2),      \
                            var3##_(i3) {invariant;}                    \
    IMP_HASHABLE_INLINE(Name, {                                         \
        std::size_t value= IMP::base::hash_value(var0##_);              \
        boost::hash_combine(value, IMP::base::hash_value(var1##_));     \
        boost::hash_combine(value, IMP::base::hash_value(var2##_));     \
        boost::hash_combine(value, IMP::base::hash_value(var3##_));     \
        return value;                                                   \
      });                                                               \
    IMP_COMPARISONS(Name);                                              \
    IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                \
                        << IMP::base::Showable(var0##_)                 \
                        << " " <<#var1 << "="                           \
                        << IMP::base::Showable(var1##_)                 \
                        << " " <<#var2 << "="                           \
                        << IMP::base::Showable(var2##_)                 \
                        << " " <<#var3 << "="                           \
                        << IMP::base::Showable(var3##_)                 \
                        << ")");                                        \
    IMP_TUPLE_VALUE(Name, type0, var0, var0##_);                        \
    IMP_TUPLE_VALUE(Name, type1, var1, var1##_);                        \
    IMP_TUPLE_VALUE(Name, type2, var2, var2##_);                        \
    IMP_TUPLE_VALUE(Name, type3, var3, var3##_);                        \
  private:                                                              \
  int compare(const Name &o) const {                                    \
    IMP_COMPARE_ONE(var0##_, o.var0##_);                                \
    IMP_COMPARE_ONE(var1##_, o.var1##_);                                \
    IMP_COMPARE_ONE(var2##_, o.var2##_);                                \
    IMP_COMPARE_ONE(var3##_, o.var3##_);                                \
    return 0;                                                           \
  }                                                                     \
  };                                                                    \
  IMP_VALUES(Name, Names)


#define IMP_NAMED_TUPLE_5(Name, Names, type0, var0, type1, var1,        \
                          type2, var2, type3, var3, type4, var4,        \
                          invariant)                                    \
  struct Name: public IMP::base::Value {                                \
  Name(type0 i0=type0(), type1 i1=type1(),type2 i2=type2(),             \
       type3 i3=type3(), type4 i4=type4()): var0##_(i0), var1##_(i1),   \
                                            var2##_(i2),                \
                                            var3##_(i3), var4##_(i4)    \
    {invariant;}                                                        \
  IMP_HASHABLE_INLINE(Name, {                                           \
      std::size_t value= IMP::base::hash_value(var0##_);                \
      boost::hash_combine(value, IMP::base::hash_value(var1##_));       \
      boost::hash_combine(value, IMP::base::hash_value(var2##_));       \
      boost::hash_combine(value, IMP::base::hash_value(var3##_));       \
      boost::hash_combine(value, IMP::base::hash_value(var4##_));       \
      return value;                                                     \
    });                                                                 \
  IMP_COMPARISONS(Name);                                                \
  IMP_SHOWABLE_INLINE(Name, out << "(" << #var0 << "="                  \
                      << IMP::base::Showable(var0##_)                   \
                      << " " <<#var1 << "="                             \
                      << IMP::base::Showable(var1##_)                   \
                      << " " <<#var2 << "="                             \
                      << IMP::base::Showable(var2##_)                   \
                      << " " <<#var3 << "="                             \
                      << IMP::base::Showable(var3##_)                   \
                      << " " <<#var4 << "="                             \
                      << IMP::base::Showable(var4##_)                   \
                      << ")");                                          \
  IMP_TUPLE_VALUE(Name, type0, var0, var0##_);                          \
  IMP_TUPLE_VALUE(Name, type1, var1, var1##_);                          \
  IMP_TUPLE_VALUE(Name, type2, var2, var2##_);                          \
  IMP_TUPLE_VALUE(Name, type3, var3, var3##_);                          \
  IMP_TUPLE_VALUE(Name, type4, var4, var4##_);                          \
  private:                                                              \
  int compare(const Name &o) const {                                    \
    IMP_COMPARE_ONE(var0##_, o.var0##_);                                \
    IMP_COMPARE_ONE(var1##_, o.var1##_);                                \
    IMP_COMPARE_ONE(var2##_, o.var2##_);                                \
    IMP_COMPARE_ONE(var3##_, o.var3##_);                                \
    IMP_COMPARE_ONE(var4##_, o.var4##_);                                \
    return 0;                                                           \
  }                                                                     \
  };                                                                    \
  IMP_VALUES(Name, Names)



/**@}*/

#endif  /* IMPBASE_TUPLE_MACROS_H */

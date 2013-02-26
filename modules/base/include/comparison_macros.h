/**
 *  \file IMP/base/comparison_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_COMPARISON_MACROS_H
#define IMPBASE_COMPARISON_MACROS_H
#include <IMP/base/base_config.h>
#include "warning_macros.h"

#if defined(IMP_DOXYGEN)
/** \name Comparisons
    Helper macros for implementing comparisons in terms of
    either member variables or a member compare function.
    All of the <,>,== etc are implemented for both C++
    and Python.
    @{
*/

//! Implement comparison in a class using a compare function
/** The compare function should take a const Name & and return -1, 0, 1 as
    appropriate.
*/
#define IMP_COMPARISONS(Name)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] field the first field to compare on
 */
#define IMP_COMPARISONS_1(Name, field)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] f0 the first field to compare on
    \param[in] f1 the second field to compare on
 */
#define IMP_COMPARISONS_2(Name, f0, f1)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] f0 the first field to compare on
    \param[in] f1 the second field to compare on
    \param[in] f2 the third field to compare on
 */
#define IMP_COMPARISONS_3(Name, f0, f1, f2)
/** @} */
#elif defined(SWIG)
#define IMP_SWIG_COMPARISONS(Name)                                      \
  bool __eq__(const Name &o) const;                                     \
  bool __ne__(const Name &o) const;                                     \
  bool __lt__(const Name &o) const;                                     \
  bool __gt__(const Name &o) const;                                     \
  bool __ge__(const Name &o) const;                                     \
  bool __le__(const Name &o) const


#define IMP_COMPARISONS(Name)                   \
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_1(Name, field)          \
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_2(Name, f0, f1)         \
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_3(Name, f0, f1, f2)     \
  IMP_SWIG_COMPARISONS(Name)

#else // not doxygen
#define IMP_SWIG_COMPARISONS(Name)                                      \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  bool __eq__(const Name &o) const {                                    \
    return operator==(o);                                               \
  }                                                                     \
  bool __ne__(const Name &o) const {                                    \
    return operator!=(o);                                               \
  }                                                                     \
  bool __lt__(const Name &o) const {                                    \
    return operator<(o);                                                \
  }                                                                     \
  bool __gt__(const Name &o) const {                                    \
    return operator>(o);                                                \
  }                                                                     \
  bool __ge__(const Name &o) const {                                    \
    return operator>=(o);                                               \
  }                                                                     \
  bool __le__(const Name &o) const {                                    \
    return operator<=(o);                                               \
  }                                                                     \
  int __cmp__(const Name &o) const {                                    \
    return compare(o);                                                  \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS                                        \
  IMP_REQUIRE_SEMICOLON_CLASS(comparisons)

#define IMP_COMPARISONS(Name)                                           \
  IMP_SWIG_COMPARISONS(Name);                                           \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  bool operator==(const Name &o) const {                                \
    return (Name::compare(o) == 0);                                     \
  }                                                                     \
  bool operator!=(const Name &o) const {                                \
    return (Name::compare(o) != 0);                                     \
  }                                                                     \
  bool operator<(const Name &o) const {                                 \
    return (Name::compare(o) <0);                                       \
  }                                                                     \
  bool operator>(const Name &o) const {                                 \
    return (compare(o) > 0);                                            \
  }                                                                     \
  bool operator>=(const Name &o) const {                                \
    return !(Name::compare(o) < 0);                                     \
  }                                                                     \
  bool operator<=(const Name &o) const {                                \
    return !(Name::compare(o) > 0);                                     \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS                                        \
  template <class T> friend int compare(const T&a, const T&b)

#define IMP_COMPARISONS_1(Name, field)                                  \
  IMP_SWIG_COMPARISONS(Name);                                           \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  bool operator==(const Name &o) const {                                \
    return (field== o.field);                                           \
  }                                                                     \
  bool operator!=(const Name &o) const {                                \
    return (field!= o.field);                                           \
  }                                                                     \
  bool operator<(const Name &o) const {                                 \
    return (field< o.field);                                            \
  }                                                                     \
  bool operator>(const Name &o) const {                                 \
    return (field> o.field);                                            \
  }                                                                     \
  bool operator>=(const Name &o) const {                                \
    return (field>= o.field);                                           \
  }                                                                     \
  bool operator<=(const Name &o) const {                                \
    return (field<= o.field);                                           \
  }                                                                     \
  int compare(const Name &o) const {                                    \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS                                         \

#define IMP_COMPARISONS_2(Name, f0, f1)                                 \
  IMP_SWIG_COMPARISONS(Name);                                           \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  bool operator==(const Name &o) const {                                \
    return (f0== o.f0 && f1==o.f1);                                     \
  }                                                                     \
  bool operator!=(const Name &o) const {                                \
    return (f0!= o.f0 || f1 != o.f1);                                   \
  }                                                                     \
  bool operator<(const Name &o) const {                                 \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    else return f1 < o.f1;                                              \
  }                                                                     \
  bool operator>(const Name &o) const {                                 \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    else return f1 > o.f1;                                              \
  }                                                                     \
  bool operator>=(const Name &o) const {                                \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  bool operator<=(const Name &o) const {                                \
    return operator<(o) || operator==(o);                               \
  }                                                                     \
  int compare(const Name &o) const {                                    \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS

#define IMP_COMPARISONS_3(Name, f0, f1, f2)                             \
  IMP_SWIG_COMPARISONS(Name);                                           \
  IMP_HELPER_MACRO_PUSH_WARNINGS                                        \
  bool operator==(const Name &o) const {                                \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);                       \
  }                                                                     \
  bool operator!=(const Name &o) const {                                \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);                     \
  }                                                                     \
  bool operator<(const Name &o) const {                                 \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    if (f1< o.f1) return true;                                          \
    else if (f1 > o.f1) return false;                                   \
    else return f2 < o.f2;                                              \
  }                                                                     \
  bool operator>(const Name &o) const {                                 \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    if (f1 > o.f1) return true;                                         \
    else if (f1 < o.f1) return false;                                   \
    else return f2 > o.f2;                                              \
  }                                                                     \
  bool operator>=(const Name &o) const {                                \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  bool operator<=(const Name &o) const {                                \
    return operator<(o) || operator==(o);                               \
  }                                                                     \
  int compare(const Name &o) const {                                    \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }                                                                     \
  IMP_HELPER_MACRO_POP_WARNINGS
#endif



#ifdef SWIG
#define IMP_SAFE_BOOL(Name, expr)
#else
/** Implement the safe bool idiom in a class. The expression must
    evaluate to a boolean.
 */
#define IMP_SAFE_BOOL(Name, expr)                               \
  private:                                                      \
  typedef void (Name::*bool_type)() const;                      \
  void this_type_does_not_support_comparisons() const {}        \
public:                                                         \
 operator bool_type() const {                                   \
 return (expr) ?                                                \
   &Name::this_type_does_not_support_comparisons : 0;           \
 }

#endif

/** Compare one value and return -1 or 1 as appriate. If they are
    equal, control returns to the current scope.
*/
#define IMP_COMPARE_ONE(vara, varb)             \
  if (vara < varb) return -1;                   \
  else if (varb < vara) return 1

#endif  /* IMPBASE_COMPARISON_MACROS_H */

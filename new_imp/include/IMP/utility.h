/**
 *  \file utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_UTILITY_H
#define __IMP_UTILITY_H


//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field) bool operator==(const This &o) const {  \
    return (field== o.field);      \
  }         \
  bool operator!=(const This &o) const {    \
    return (field!= o.field);      \
  }         \
  bool operator<(const This &o) const {     \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field< o.field);      \
  }         \
  bool operator>(const This &o) const {     \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field> o.field);      \
  }         \
  bool operator>=(const This &o) const {    \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field>= o.field);      \
  }         \
  bool operator<=(const This &o) const {    \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field<= o.field);      \
  }

//! Implement operator<< on class name, assuming it has one template argument
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_1(name) template <class L>                  \
  inline std::ostream &operator<<(std::ostream &out, const name<L> &i)  \
  {                                                                     \
    return i.show(out);                                                 \
  }

//! Implement operator<< on class name
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR(name)                                    \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    return i.show(out);                                                 \
  }

#define IMP_HIDE(x) x

#endif  /* __IMP_UTILITY_H */

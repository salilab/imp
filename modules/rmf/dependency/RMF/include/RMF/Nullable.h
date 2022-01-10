/**
 *  \file RMF/Nullable.h
 *  \brief A helper class for allowing nice return of possibly null values.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NULLABLE_H
#define RMF_NULLABLE_H

#include "RMF/config.h"
#include "traits.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include <limits>

RMF_ENABLE_WARNINGS

namespace RMF {

/** \brief Return a possibly null value.

  Return a value as returned by RMF with the additional ability to check if it
  is the null value. These get converted to plain values or `None` in python.
  \note they should never be stored.
*/
template <class T>
class Nullable {
  typename Traits<T>::ReturnType v_;
  std::string get_string() const {
    if (get_is_null())
      return "<null>";
    else {
      std::ostringstream oss;
      oss << v_;
      return oss.str();
    }
  }

 public:
  Nullable(const Nullable& o) : v_(o.v_) {}
#ifndef SWIG
  Nullable(typename Traits<T>::ReturnType v) : v_(v) {}
  /** \pre !get_is_null() */
  operator typename Traits<T>::ReturnType() const { return get(); }
  /** \pre !get_is_null() */
  typename Traits<T>::ReturnType get() const {
    RMF_USAGE_CHECK(!get_is_null(), "Can't convert null value.");
    return v_;
  }
#else
  T get() const;
#endif
  bool get_is_null() const { return Traits<T>::get_is_null_value(v_); }

#ifndef IMP_DOXYGEN
  /** For python since it nicely becomes None. */
  const T* get_ptr() const {
    if (get_is_null())
      return NULL;
    else
      return &v_;
  }
#endif
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  void show(std::ostream& out) const { out << get_string(); }
#endif
};

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
template <class Traits>
inline std::ostream& operator<<(std::ostream& out, Nullable<Traits> null) {
  null.show(out);
  return out;
}
#endif

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NULLABLE_H */

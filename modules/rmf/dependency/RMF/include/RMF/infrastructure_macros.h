/**
 *  \file RMF/infrastructure_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INFRASTRUCTURE_MACROS_H
#define RMF_INFRASTRUCTURE_MACROS_H

#include <sstream>
#include <iostream>
#include <string>
#include "RMF/compiler_macros.h"
#include "internal/use.h"
#include <vector>
#include <boost/config.hpp>
#include <boost/version.hpp>
#if defined(BOOST_NO_CXX11_RANGE_BASED_FOR) || BOOST_VERSION < 105300
#define RMF_FOREACH(v, r) BOOST_FOREACH(v, r)
#include <boost/foreach.hpp>  // IWYU pragma: export
#else
#define RMF_FOREACH(v, r) for (v : r)
#endif

RMF_ENABLE_WARNINGS

#ifdef NDEBUG
#define RMF_NDEBUG
#endif

#if defined(RMF_DOXYGEN)
/** \name Comparisons
    Helper macros for implementing comparisons in terms of
    either member variables or a member compare function.
    All of the <,>,== etc are implemented for both C++
    and Python.
    @{
 */

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
 */
#define RMF_COMPARISONS(Name)

/** @} */
#elif defined(SWIG)
#define RMF_SWIG_COMPARISONS(Name)  \
  bool __eq__(const Name& o) const; \
  bool __ne__(const Name& o) const; \
  bool __lt__(const Name& o) const; \
  bool __gt__(const Name& o) const; \
  bool __ge__(const Name& o) const; \
  bool __le__(const Name& o) const

#define RMF_COMPARISONS(Name) RMF_SWIG_COMPARISONS(Name)

#else  // not doxygen

#define RMF_SWIG_COMPARISONS(Name)                           \
  bool __eq__(const Name& o) const { return operator==(o); } \
  bool __ne__(const Name& o) const { return operator!=(o); } \
  bool __lt__(const Name& o) const { return operator<(o); }  \
  bool __gt__(const Name& o) const { return operator>(o); }  \
  bool __ge__(const Name& o) const { return operator>=(o); } \
  bool __le__(const Name& o) const { return operator<=(o); } \
  int __cmp__(const Name& o) const { return compare(o); }

#define RMF_COMPARISONS(Name)                                      \
  bool operator==(const Name& o) const { return compare(o) == 0; } \
  bool operator!=(const Name& o) const { return compare(o) != 0; } \
  bool operator<(const Name& o) const { return compare(o) < 0; }   \
  bool operator>(const Name& o) const { return compare(o) > 0; }   \
  bool operator>=(const Name& o) const { return compare(o) >= 0; } \
  bool operator<=(const Name& o) const { return compare(o) <= 0; } \
  RMF_SWIG_COMPARISONS(Name)

#endif

#ifdef RMF_DOXYGEN

//! Implement a hash function for the class
#define RMF_HASHABLE(name, hashret)
/** @} */
#else

#define RMF_HASHABLE(name, hashret) \
  std::size_t __hash__() const { hashret; }

#endif

/** @} */

#ifdef SWIG
#define RMF_SHOWABLE(Name, streamed) \
  std::string __str__() const {      \
    std::ostringstream out;          \
    show(out);                       \
    return out.str();                \
  }                                  \
  std::string __repr__() const {     \
    std::ostringstream out;          \
    show(out);                       \
    return out.str();                \
  }

#elif defined(RMF_DOXYGEN)
#define RMF_SHOWABLE(Name, streamed)

#else
#define RMF_SHOWABLE(Name, streamed)                      \
  operator Showable() const {                             \
    std::ostringstream oss;                               \
    oss << streamed;                                      \
    return Showable(oss.str(), Showable::Special());      \
  }                                                       \
  void show(std::ostream& out) const { out << streamed; } \
  std::string __str__() const {                           \
    std::ostringstream out;                               \
    show(out);                                            \
    return out.str();                                     \
  }                                                       \
  std::string __repr__() const {                          \
    std::ostringstream out;                               \
    show(out);                                            \
    return out.str();                                     \
  }
#endif

/** Suppress the unused variable compiler warning. */
#define RMF_UNUSED(variable) RMF::internal::use(variable);

/** Provide a dummy return for something that doesn't return.

    This is a nasty hack to suppress warnings in some compilers.
*/
#define RMF_NO_RETURN(type) return type()

/** Apply the macro to each supported constant size type (e.g. int as opposed
    to string).

    \see RMF_FOREACH_TYPE()
 */
#define RMF_FOREACH_SIMPLE_TYPE(macroname)                      \
  macroname(int, Int, int, int, const Ints&, Ints);             \
  macroname(float, Float, float, float, const Floats&, Floats); \
  macroname(index, Index, int, int, const Ints&, Ints)

/** Expand to applying the macro to each type supported by
    the RMF library. The macro should take six arguments
    - the lower case name of the type
    - the upper case name
    - the C++ type for accepting the value
    - the C++ type for returning the value
    - the C++ type for accepting more than one value
    - the C++ type for returning more than one value
*/
#define RMF_FOREACH_TYPE(macroname)                                         \
  macroname(RMF::IntTraits, Int) macroname(RMF::FloatTraits, Float)         \
      macroname(RMF::StringTraits, String) macroname(RMF::IntsTraits, Ints) \
      macroname(RMF::FloatsTraits, Floats)                                  \
      macroname(RMF::StringsTraits, Strings)                                \
      macroname(RMF::Vector3Traits, Vector3)                                \
      macroname(RMF::Vector4Traits, Vector4)                                \
      macroname(RMF::Vector3sTraits, Vector3s)

#define RMF_DECORATOR_CATCH(extra_info)                              \
  catch (Exception& e) {                                             \
    RMF_RETHROW(Decorator(get_decorator_type_name()) extra_info, e); \
  }

/** Register a validator function. See Validator for more
    information.*/
#define RMF_VALIDATOR(Type) RMF::Registrar<Type> Type##Reg(#Type);
namespace RMF {

template <class Tr>
class Nullable;

#if !defined(RMF_DOXYGEN) && !defined(SWIG)
struct Showable;
inline std::ostream& operator<<(std::ostream& out, const Showable& t);

/** An adaptor class to provide operator<< for classes easily. */
struct Showable {
  std::string t_;
  template <class T>
  explicit Showable(const T& t) {
    std::ostringstream oss;
    oss << t;
    t_ = oss.str();
  }
  template <class T, class TT>
  Showable(const std::pair<T, TT>& p) {
    std::ostringstream oss;
    oss << "(" << p.first << ", " << p.second << ")";
    t_ = oss.str();
  }
  Showable(std::string t) : t_(std::string("\"") + t + "\"") {}
  struct Special {};
  Showable(std::string t, Special) : t_(t) {}
  template <class T>
  Showable(const std::vector<T>& t) {
    std::ostringstream out;
    out << "[";
    for (unsigned int i = 0; i < t.size(); ++i) {
      if (i != 0) {
        out << ", ";
      }
      out << t[i];
    }
    out << "]";
    t_ = out.str();
  }
  template <class Tr>
  Showable(const Nullable<Tr>& t) {
    std::ostringstream out;
    if (t.get_is_null())
      out << "<None>";
    else
      out << t.get();
    t_ = out.str();
  }
};

inline std::ostream& operator<<(std::ostream& out, const Showable& t) {
  out << t.t_;
  return out;
}

#endif
}

RMF_DISABLE_WARNINGS

#endif /* RMF_INFRASTRUCTURE_MACROS_H */

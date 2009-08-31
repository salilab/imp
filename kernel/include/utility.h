/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_UTILITY_H
#define IMP_UTILITY_H

#include "macros.h"
#include "config.h"
#include <boost/version.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <algorithm>

#if BOOST_VERSION >= 103500
#include <boost/math/special_functions/fpclassify.hpp>
#else
#ifdef __GNUC__
#include <cmath>
#endif // __GNUC__
#endif // BOOST_VERSION

IMP_BEGIN_NAMESPACE

//! Compute the square of a number
template <class T>
T square(T t)
{
  return t*t;
}

//! Compute the cube of a number
template <class T>
T cube(T t)
{
  return t*t*t;
}


//! Return true if a number is NaN
/** With certain compiler settings the compiler can optimize
 out a!=a (and certain intel chips had issues with it too).
 */
template <class T>
inline bool is_nan(const T& a) {
#if BOOST_VERSION >= 103500
  return (boost::math::isnan)(a);
#else
  // Not all gcc versions include C99 math
#if defined(_GLIBCXX_USE_C99_MATH) && defined(__GNUC__)
  return (std::isnan)(a);
#else
  return a != a;
#endif // C99
#endif // BOOST_VERSION
}



//! A version of std::for_each which works with ranges
/** This is needed to apply the functor to a range which is a temporary
    object, since you can't call both begin and end on it.
 */
template <class Range, class Functor>
void for_each(const Range &r, const Functor &f) {
  std::for_each(r.begin(), r.end(), f);
}

/** \brief A base class to declare that a given class is initialized a
    valid stated by the default constructor.

    Instances of a class inhertiting from IMP::ValidDefault are put into
    a well defined, valid state by the default constructor.
    \see NullDefault
    \see UninitializedDefault
    */
struct ValidDefault{};

/** \brief A base class to declare that a given class is initialized to a
    null state by the dafault constructor.

    By inheriting from NullDefault, a class asserts that its default
    constructed value defines a well defined, unique, invalid value
    which can be used for comparisons. Such a value should have the
    same semantics as NULL for pointers or 0 for integers. Default
    constructed instances can be compared against, but all other operations
    on them should be assumed to be invalid unless documented otherwise.
    \code
    NullDefault() == NullDefault()
    \endcode

    \see UninitializedDefault
    \see ValidDefault
*/
struct NullDefault{};

/** \brief A base class to declare that the default constructor of the
    class puts it in an unknown invalid state.

    Classes that inherit from NullDefault are put into an unknown state
    by their default constructor. Such classes mimic
    POD types (int, float etc) in C++ and are optimized for efficiency.
    All operations on a default initialized instance other than assigning
    to it from a non-default initialized instance should be assumed to be
    invalid.
    \code
    UninitializeDefault() != UninitializedDefault()
    \endcode
    \see NullDefault
    \see ValidDefault
*/
struct UninitializedDefault{};


/** \brief A base class to declare that the object can be compared against
    other objects of the same type.

    Objects inheriting from IMP::Comparable should support the operators
    \c <, \c >, \c ==, \c !=, \c >=, \c <= against other objects of the same
    type. In addition they support a namespace function \c compare(a,b) which
    return -1,0,1 if \c a \c < \c b, \c a\c == \c b or \c a \c > \c b.
*/
struct Comparable {};


template <class T>
int compare(const T &a, const T &b) {
  return a.compare(b);
}


/** \brief A base class which provides a warning if the object is never passed
    to another before being destroyed.

    This base class provides an automatic check to make sure that Restraint
    or similar objects actually get added to the model. It will print a warning
    if they are not. To turn off this warning for a particular object, call
    the set_was_owned() method with true.
*/
class IMPEXPORT Ownable {
  bool was_owned_;
public:
  Ownable(): was_owned_(true){}
  void set_was_owned(bool tf) {
    was_owned_=tf;
  }
  ~Ownable();
};


/** \brief RAII-style objects

    RAII-style are a convenient way of controlling a resource. They assume
    "ownership" of the resource on creation and then "free" it on destruction.
    Examples include, setting the log level to a particular value and restoring
    the old value when the object goes out of scope (SetLogState) or
    incrementing and decrementing a ref-counted pointer (Pointer).

    RAII objects should either take no arguments and just have a constructor and
    destructor or should have a constructor, destructor and set and reset
    functions.
 */
class RAII IMP_NO_SWIG(: public boost::noncopyable) {
#ifdef DOXYGEN
  RAII(args);
  void set(args);
  void reset();
  ~RAII();
#endif
};

IMP_END_NAMESPACE

#endif  /* IMP_UTILITY_H */

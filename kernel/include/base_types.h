/**
 *  \file IMP/base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_BASE_TYPES_H
#define IMP_BASE_TYPES_H

#include "kernel_config.h"
#include "Key.h"
#include "macros.h"
#include <string>
#include <vector>
#include <IMP/compatibility/checked_vector.h>

IMP_BEGIN_NAMESPACE

/** \internal \namespace IMP::internal Implementation details.
 */

//! Basic floating-point value (could be float, double...)
typedef double Float;

//! A pair representing a function value with its first derivative
typedef std::pair<double, double> DerivativePair;
IMP_BUILTIN_VALUES(DerivativePair, DerivativePairs);

//! A generic pair of floats
typedef std::pair<double, double> FloatPair;
IMP_BUILTIN_VALUES(FloatPair, FloatPairs);

//! A pair representing the allowed range for a Float attribute
typedef std::pair<Float, Float> FloatRange;
IMP_BUILTIN_VALUES(FloatRange, FloatRanges);

//! Basic integer value
typedef int Int;

typedef std::pair<Int, Int> IntRange;
IMP_BUILTIN_VALUES(IntRange, IntRanges);

typedef std::pair<Int, Int> IntPair;
IMP_BUILTIN_VALUES(IntPair, IntPairs);


//! Basic string value
typedef std::string String;

//! Standard way to pass a bunch of Float values
IMP_BUILTIN_VALUES(Float, Floats);
//! Standard way to pass a bunch of Int values
IMP_BUILTIN_VALUES(Int, Ints);
//! Standard way to pass a bunch of String values
IMP_BUILTIN_VALUES(String, Strings);

//! Standard way to pass a bunch of Floats values
IMP_BUILTIN_VALUES(Floats, FloatsList);
//! Standard way to pass a bunch of Ints values
IMP_BUILTIN_VALUES(Ints, IntsList);
//! Standard way to pass a bunch of Strings values
IMP_BUILTIN_VALUES(Strings, StringsList);

/** @name Attribute Keys
    Each type of attribute has an associated type of key. The keys can
    be constructed from a string. Such construction can be expensive
    and so the resulting keys should be cached.
    @{
 */

//! The type used to identify float attributes in the Particles
IMP_DECLARE_KEY_TYPE(FloatKey, 0);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntKey, 1);
//! The type used to identify string attributes in the Particles
IMP_DECLARE_KEY_TYPE(StringKey, 2);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticleKey, 3);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectKey, 4);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntsKey, 5);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticlesKey, 6);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectsKey, 7);

/** @} */

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline void show(std::ostream &out, int i) {
  out << i;
}

inline void show(std::ostream &out, unsigned int i) {
  out << i;
}

inline void show(std::ostream &out, std::string i) {
  out << i;
}

inline void show(std::ostream &out, double i) {
  out << i;
}
template <class T>
inline void show(std::ostream &out, const T& i) {
  out << i;
}
template <class T>
inline void show(std::ostream &out, const T*i) {
  out << i->get_name();
}
template <class T>
inline void show(std::ostream &out, T*i) {
  out << i->get_name();
}
#endif


#ifndef IMP_DOXYGEN
template <class T>
inline std::size_t hash_value(const T &t) {
  return t.__hash__();
}




#endif

#ifdef IMP_DOXYGEN
/** The C++0x standard adds the nullptr keyword to get around a variety of
    problems with NULL. We provide an emulation within the IMP namespace when
    it is not available.
*/
const std::nullptr_t nullptr;

#else

#if IMP_DEFINE_NULLPTR

#if !defined(SWIG)
struct nullptr_t {
  template <class O>
  operator O*() const {
    return static_cast<O*>(NULL);
  }
  /*template <class O, class C>
  operator O C::*() const {
    return static_cast<const O*>(NULL);
    }*/
};
template <class O>
inline bool operator==(O *a, nullptr_t o) {
  return a == static_cast<O*>(o);
}
template <class O>
inline bool operator!=(O *a, nullptr_t o) {
  return a != static_cast<O*>(o);
}
extern IMPEXPORT const nullptr_t nullptr;
#else
extern const void * const nullptr;
#endif //SWIG

#endif // IMP_DEFINE_NULLPTR
#endif // IMP_DOXYGEN

IMP_END_NAMESPACE

#endif  /* IMP_BASE_TYPES_H */

/**
 *  \file RMF/traits.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_TRAITS_H
#define RMF_TRAITS_H

#include "RMF/config.h"
#include "RMF/types.h"
#include <boost/cstdint.hpp>
#include <limits>
#include <cmath>
#if RMF_HAS_DEPRECATED_BACKENDS
#include "RMF/HDF5/types.h"
#endif

RMF_ENABLE_WARNINGS

namespace RMF {

#ifndef SWIG
//! Traits classes describing how RMF uses types for storing data
/** Traits classes that describe how types are handled. Currently, adding a new
    type is beyond the scope of the documentation, so traits is not documented.
*/
template <class T>
struct Traits {};

template <class T>
struct ValueTraitsBase {
  typedef T Type;
  typedef std::vector<T> Types;
  typedef T ArgumentType;
  typedef T ReturnType;
  static bool get_are_equal(T a, T b) { return a == b; }
};

template <class T>
struct ReferenceTraitsBase {
  typedef T Type;
  typedef std::vector<T> Types;
  typedef const T& ArgumentType;
  typedef const T& ReturnType;
};

template <>
struct Traits<Int> : public ValueTraitsBase<Int> {
  static bool get_is_null_value(const Type& t) { return t == get_null_value(); }
  static ReturnType get_null_value() {
    return std::numeric_limits<Type>::max();
  }
#if RMF_HAS_DEPRECATED_BACKENDS
  typedef HDF5::IntTraits HDF5Traits;
  typedef boost::int32_t AvroType;
#endif
  static std::string get_tag() { return "ki"; }
  static std::string get_name() { return "int"; }
};

template <>
struct Traits<Float> : public ValueTraitsBase<Float> {
  static bool get_is_null_value(const Type& t) {
    return t >= std::numeric_limits<Float>::max();
  }
  static ReturnType get_null_value() {
    return std::numeric_limits<Type>::infinity();
  }
#if RMF_HAS_DEPRECATED_BACKENDS
  typedef HDF5::FloatTraits HDF5Traits;
  typedef double AvroType;
#endif
  static bool get_are_equal(ArgumentType a, ArgumentType b) {
    return std::abs(a - b) < .0000001 * std::abs(a + b) + .000000001;
  }
  static std::string get_tag() { return "kf"; }
  static std::string get_name() { return "float"; }
};

template <>
struct Traits<String> : public ValueTraitsBase<String> {
  static bool get_is_null_value(const Type& t) { return t.empty(); }
  static ReturnType get_null_value() {
    static Type r;
    return r;
  }
#if RMF_HAS_DEPRECATED_BACKENDS
  typedef HDF5::StringTraits HDF5Traits;
  typedef Type AvroType;
#endif
  static std::string get_tag() { return "ks"; }
  static std::string get_name() { return "string"; }
};

template <class T>
struct SequenceTraitsBase {
  typedef std::vector<T> Type;
  typedef std::vector<Type> Types;
  typedef Type ReturnType;
  typedef const Type& ArgumentType;
  static bool get_is_null_value(const Type& t) { return t.empty(); }
  static ReturnType get_null_value() {
    static Type r;
    return r;
  }
  static bool get_are_equal(ArgumentType a, ArgumentType b) {
    if (a.size() != b.size()) return false;
    for (unsigned int i = 0; i < a.size(); ++i) {
      if (!Traits<T>::get_are_equal(a[i], b[i])) return false;
    }
    return true;
  }
  static std::string get_tag() { return Traits<T>::get_tag() + "s"; }
  static std::string get_name() { return Traits<T>::get_name() + "s"; }
};

template <>
struct Traits<Ints> : public SequenceTraitsBase<Int> {
#if RMF_HAS_DEPRECATED_BACKENDS
  typedef HDF5::IntsTraits HDF5Traits;
  typedef std::vector<Traits<Int>::AvroType> AvroType;
#endif
};

template <>
struct Traits<Floats> : public SequenceTraitsBase<Float> {
#if RMF_HAS_DEPRECATED_BACKENDS
  typedef HDF5::FloatsTraits HDF5Traits;
  typedef std::vector<double> AvroType;
#endif
};

template <>
struct Traits<Strings> : public SequenceTraitsBase<String> {
#if RMF_HAS_DEPRECATED_BACKENDS
  typedef HDF5::StringsTraits HDF5Traits;
  typedef Type AvroType;
#endif
};

template <unsigned int D>
class Traits<Vector<D> > : public ReferenceTraitsBase<Vector<D> > {
  static std::string make_tag() {
    std::ostringstream oss;
    oss << "v" << D;
    return oss.str();
  }
  static std::string make_name() {
    std::ostringstream oss;
    oss << "vector" << D;
    return oss.str();
  }

 public:
  static bool get_is_null_value(const RMF_VECTOR<D>& t) {
    return t[0] > std::numeric_limits<double>::max();
  }
  static const RMF_VECTOR<D>& get_null_value() {
    static const RMF_VECTOR<D> null(Floats(
        D, std::numeric_limits<typename Traits<Float>::Type>::infinity()));
    return null;
  }
  static bool get_are_equal(const RMF_VECTOR<D>& a, const RMF_VECTOR<D>& b) {
    for (unsigned int i = 0; i < D; ++i) {
      if (!Traits<Float>::get_are_equal(a[i], b[i])) return false;
    }
    return true;
  }
  static std::string get_tag() {
    static std::string tag = make_tag();
    return tag;
  }
  static std::string get_name() {
    static std::string name = make_name();
    return name;
  }
};

template <unsigned int D>
struct Traits<std::vector<Vector<D> > > : public SequenceTraitsBase<
                                              Vector<D> > {};

typedef Traits<Int> IntTraits;
typedef Traits<Float> FloatTraits;
typedef Traits<String> StringTraits;
typedef Traits<Ints> IntsTraits;
typedef Traits<Floats> FloatsTraits;
typedef Traits<Strings> StringsTraits;
typedef Traits<Vector<3> > Vector3Traits;
typedef Traits<Vector<4> > Vector4Traits;
typedef Traits<std::vector<Vector<3> > > Vector3sTraits;
typedef Traits<std::vector<Vector<4> > > Vector4sTraits;

/*struct IntTag : public IntTraits {};
struct FloatTag : public FloatTraits {};
struct StringTag : public StringTraits {};
struct Vector3Tag : public Vector3Traits {};
struct Vector4Tag : public Vector4Traits {};
struct IntsTag : public IntsTraits {};
struct FloatsTag : public FloatsTraits {};
struct StringsTag : public StringsTraits {};
struct Vector3sTag : public Vector3sTag {};
struct Vector4sTag : public Vector4sTraits {};*/
typedef IntTraits IntTag;
typedef FloatTraits FloatTag;
typedef StringTraits StringTag;
typedef Vector3Traits Vector3Tag;
typedef Vector4Traits Vector4Tag;
typedef IntsTraits IntsTag;
typedef FloatsTraits FloatsTag;
typedef StringsTraits StringsTag;
typedef Vector3sTraits Vector3sTag;
typedef Vector4sTraits Vector4sTag;

#else
struct IntTag {};
struct FloatTag {};
struct StringTag {};
struct Vector3Tag {};
struct Vector4Tag {};
struct IntsTag {};
struct FloatsTag {};
struct StringsTag {};
struct Vector3sTag {};
struct Vector4sTag {};
#endif

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_TRAITS_H */

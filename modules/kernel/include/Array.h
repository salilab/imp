/**
 *  \file IMP/Array.h
 *  \brief Classes to handle static sized arrays of things.
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_ARRAY_H
#define IMPKERNEL_ARRAY_H

#include <IMP/kernel_config.h>
#include "Value.h"
#include "comparison_macros.h"
#include "hash_macros.h"
#include "check_macros.h"
#include "showable_macros.h"
#include <array>
#include <type_traits>
#include <cereal/access.hpp>
#include <cereal/types/array.hpp>

IMPKERNEL_BEGIN_NAMESPACE

//! A class to store a fixed array of same-typed values.
/** Only the constructor with the correct number of arguments for the
    dimensionality can be used.

    Elements can be accessed using [] notation or std::get. The latter
    is more efficient when the index is a constant, since the bounds check
    can be done at compile time rather than runtime, e.g. x = std::get<0>(array)
    is more efficient than x = array[0].

    \note These are mapped to/from Python tuples, so there is
    no need to use types that are typedefs of this on the Python side.

    \see ConstVector
 */
template <unsigned int D, class Data, class SwigData = Data>
class Array : public Value {
  typedef std::array<Data, D> Storage;
  Storage d_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(d_);
  }

  int compare(const Array<D, Data, SwigData>& o) const {
    for (unsigned int i = 0; i < D; ++i) {
      if (d_[i] < o.get(i))
        return -1;
      else if (d_[i] > o.get(i))
        return 1;
    }
    return 0;
  }

 public:
#ifndef IMP_DOXYGEN
  typedef SwigData value_type;
#endif
  unsigned int get_dimension() {
    return D;
  };
  Array() {}

  template<int DT=D, typename std::enable_if<DT == 2>::type* = nullptr>
  Array(SwigData x, SwigData y) {
    d_[0] = x;
    d_[1] = y;
  }

  template<int DT=D, typename std::enable_if<DT == 3>::type* = nullptr>
  Array(SwigData x, SwigData y, SwigData z) {
    d_[0] = x;
    d_[1] = y;
    d_[2] = z;
  }

  template<int DT=D, typename std::enable_if<DT == 4>::type* = nullptr>
  Array(SwigData x0, SwigData x1, SwigData x2, SwigData x3) {
    d_[0] = x0;
    d_[1] = x1;
    d_[2] = x2;
    d_[3] = x3;
  }

  SwigData get(unsigned int i) const { return d_[i]; }

  IMP_HASHABLE_INLINE(Array, std::size_t seed = 0;
                      for (unsigned int i = 0; i < D; ++i) {
                             boost::hash_combine(seed, d_[i]);
                           } return seed;);
  IMP_COMPARISONS(Array);
#ifndef SWIG
  const Data operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < D, "Out of range");
    return d_[i];
  }
  Data& operator[](unsigned int i) {
    IMP_USAGE_CHECK(i < D, "Out of range");
    return d_[i];
  }
#ifndef IMP_DOXYGEN
  void set_item(unsigned int i, SwigData v) const {
    IMP_USAGE_CHECK(i < D, "Out of range");
    d_[i] = v;
  }
#endif
#endif
#ifndef IMP_DOXYGEN
  SwigData __getitem__(unsigned int i) const {
    if (i >= D) IMP_THROW("Out of bound " << i << " vs " << D, IndexException);
    return operator[](i);
  }
  unsigned int __len__() const { return D; }
#endif
#ifndef SWIG
  unsigned int size() const { return D; }
#endif
  std::string get_name() const {
    std::ostringstream oss;
    oss << "\"";
    for (unsigned int i = 0; i < D; ++i) {
      if (i > 0) {
        oss << "\" and \"";
      }
      oss << d_[i];
    }
    oss << "\"";
    return oss.str();
  }
  IMP_SHOWABLE_INLINE(Array, { out << get_name(); });
  typedef typename Storage::iterator iterator;
  iterator begin() { return d_.begin(); }
  iterator end() { return d_.end(); }
  typedef typename Storage::const_iterator const_iterator;
  const_iterator begin() const { return d_.begin(); }
  const_iterator end() const { return d_.end(); }

  Data* data() {
    if (d_.empty()) {
      return NULL;
    } else {
      return &d_.front();
    }
  }

  const Data* data() const {
    if (d_.empty()) {
      return NULL;
    } else {
      return &d_.front();
    }
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <unsigned int D, class Data, class SwigData>
inline std::size_t hash_value(const Array<D, Data, SwigData> &t) {
  return t.__hash__();
}
#endif

IMPKERNEL_END_NAMESPACE

/* Overload std::get to work on IMP::Array similarly to std::array */
namespace std {
  template <unsigned int I, unsigned int D, class Data, class SwigData>
  const Data& get(const IMP::Array<D, Data, SwigData> &arr) {
    static_assert(I < D, "array index is within bounds");
    return *(arr.begin() + I);
  }
}

#endif /* IMPKERNEL_ARRAY_H */

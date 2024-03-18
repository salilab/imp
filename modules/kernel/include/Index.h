/**
 *  \file IMP/Index.h
 *  \brief Utility types to refer to various types of indices
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INDEX_H
#define IMPKERNEL_INDEX_H

#include <IMP/kernel_config.h>
#include "bracket_macros.h"
#include "showable_macros.h"
#include "Value.h"
#include <cereal/access.hpp>
#include <functional>
#include <IMP/Vector.h>

IMPKERNEL_BEGIN_NAMESPACE

//! A typed index.
/** This can help disambiguate different integer based indexes floating
    around to help avoid bugs caused by mixing them up. Care has been taken
    so that it can be replaced by an integer everywhere, if needed. */
template <class Tag>
class Index {
  // We should really derive from Value, but this seems to confuse gcc at least
  // into padding the struct and wasting memory,
  // e.g. sizeof(ParticleIndexPair) should be 8 (2*int) but is actually 12.
  int i_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(i_);
  }

 public:
  explicit Index(int i) : i_(i) {}
  Index() : i_(-2) {}
  int get_index() const {
    IMP_INTERNAL_CHECK(i_ != -2, "Uninitialized index");
    IMP_INTERNAL_CHECK(i_ >= 0, "Invalid index");
    return i_;
  }
  IMP_COMPARISONS_1(Index, i_);
  IMP_SHOWABLE_INLINE(Index, {
    IMP_INTERNAL_CHECK(i_ != -2, "Uninitialized index");
    out << i_;
  });
  IMP_HASHABLE_INLINE(Index, {
    IMP_INTERNAL_CHECK(i_ != -2, "Uninitialized index");
    return i_;
  });
};
template <class Tag>
inline unsigned int get_as_unsigned_int(Index<Tag> i) {
  return i.get_index();
}
template <class Tag>
inline Index<Tag> get_invalid_index() {
  return Index<Tag>(-1);
}

// Compression types for serialization of IndexVector.
// Since these vectors are often sparse, we do basic compression on
// the serialized data. The data is written out in a number of blocks,
// each starting with one of these compression types, until we hit COMP_END.
namespace {
  // No compression: this is followed by a size N and then N values
  static const unsigned char COMP_NONE = 0;

  // Simple run-length encoding: followed by a size N and a single value
  static const unsigned char COMP_RLE = 1;

  // More compression types can be added here...

  // End of serialization: we are done serializing this vector
  static const unsigned char COMP_END = 100;
}

//! Implements a vector tied to a particular index of type Index<Tag>.
/** When this class is serialized, the output data are compressed using simple
    run-length encoding, as these vectors are often sparse. For objects that
    do not implement operator== (e.g. VectorD, SphereD), a custom comparison
    functor should be provided. */
template <class Tag, class T, class Allocator = std::allocator<T>,
          class Equal = std::equal_to<T> >
class IndexVector : public Vector<T, Allocator> {
  typedef Vector<T, Allocator> P;

  template<class Archive> void write_no_compression(
                       Archive &ar, typename P::const_iterator start,
                       typename P::const_iterator end) const {
    size_t sz = end - start;
    ar(COMP_NONE); ar(sz);
    while(start != end) {
      ar(*start++);
    }
  }

  template<class Archive> void write_rle(
                       Archive &ar, typename P::const_iterator start,
                       typename P::const_iterator end) const {
    size_t sz = end - start;
    ar(COMP_RLE); ar(sz);
    ar(*start);
  }

  friend class cereal::access;
  template<class Archive> void save(Archive &ar) const {
    size_t sz = P::size();
    ar(sz);
    typename P::const_iterator pos = P::begin(), start = P::begin(), runend;
    Equal cmp;
    while (pos != P::end()) {
      const T& val = *pos;
      // update runend to point past the end of a run of same values,
      // starting at pos
      for (runend = pos + 1; runend != P::end() && cmp(*runend, val);
           ++runend) {}
      // exclude very short runs
      if (runend - pos > 10) {
        if (pos > P::begin() && pos > start) {
          // Write previous set of non-RLE values
          write_no_compression(ar, start, pos);
        }
        write_rle(ar, pos, runend);
        start = runend;
      }
      pos = runend;
    }
    if (start != P::end()) {
      write_no_compression(ar, start, P::end());
    }
    ar(COMP_END);
  }

  // Read both non-compressed and compressed serialized streams
  template<class Archive> void load(Archive &ar) {
    P::resize(0);
    size_t sz;
    ar(sz);
    P::reserve(sz);
    unsigned char comp_type;
    ar(comp_type);
    while(comp_type != COMP_END) {
      if (comp_type == COMP_NONE) {
        ar(sz);
        while(sz-- > 0) {
          T val;
          ar(val);
          P::push_back(val);
        }
      } else if (comp_type == COMP_RLE) {
        ar(sz);
        T val;
        ar(val);
        while(sz-- > 0) {
          P::push_back(val);
        }
      } else {
        IMP_THROW("Unsupported IndexVector compression type", ValueException);
      }
      ar(comp_type);
    }
  }

 public:
  IndexVector(unsigned int sz, const T &t = T()) : P(sz, t) {}
  IndexVector() {}
  IMP_BRACKET(T, Index<Tag>, get_as_unsigned_int(i) < P::size(),
              return P::operator[](get_as_unsigned_int(i)));
};

template <class Tag, class Container, class T>
void resize_to_fit(Container &v, Index<Tag> i, const T &default_value = T()) {
  if (v.size() <= get_as_unsigned_int(i)) {
    v.resize(get_as_unsigned_int(i) + 1, default_value);
  }
}

IMPKERNEL_END_NAMESPACE

namespace cereal {
  template <class Archive, class Tag, class T>
  struct specialize<Archive, IMP::IndexVector<Tag, T>,
                    cereal::specialization::member_load_save> {};
}

#endif /* IMPKERNEL_INDEX_H */

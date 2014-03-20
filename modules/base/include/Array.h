/**
 *  \file IMP/base/Array.h
 *  \brief Classes to handle static sized arrays of things.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_ARRAY_H
#define IMPBASE_ARRAY_H

#include <IMP/base/base_config.h>
#include "Value.h"
#include "comparison_macros.h"
#include "hash_macros.h"
#include "check_macros.h"
#include "showable_macros.h"
#include <boost/array.hpp>

IMPBASE_BEGIN_NAMESPACE

//! A class to store an fixed array of same-typed values.
/** Only the constructor with the correct number of arguments for the
        dimensionality can be used.

        \note These are mapped to/from python tuples, so there is
        no need to use types that are typedefs of this on the python
        side.

        \see ConstVector
    */
template <unsigned int D, class Data, class SwigData = Data>
class Array : public Value {
  typedef boost::array<Data, D> Storage;
  Storage d_;
  int compare(const Array<D, Data, SwigData>& o) const {
    for (unsigned int i = 0; i < D; ++i) {
      if (d_[i] < o[i])
        return -1;
      else if (d_[i] > o[i])
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
  Array(SwigData x, SwigData y) {
    IMP_USAGE_CHECK(D == 2, "Need " << D << " to construct a " << D
                                    << "-tuple.");
    d_[0] = x;
    d_[1] = y;
  }
  Array(SwigData x, SwigData y, SwigData z) {
    IMP_USAGE_CHECK(D == 3, "Need " << D << " to construct a " << D
                                    << "-tuple.");
    d_[0] = x;
    d_[1] = y;
    d_[2] = z;
  }
  Array(SwigData x0, SwigData x1, SwigData x2, SwigData x3) {
    IMP_USAGE_CHECK(D == 4, "Need " << D << " to construct a " << D
                                    << "-tuple.");
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
};

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_ARRAY_H */

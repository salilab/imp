/**
 *  \file MinimalSet.h    \brief Maintain a set of low scoring elements.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_INTERNAL_MINIMAL_SET_H
#define IMPALGEBRA_INTERNAL_MINIMAL_SET_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/Vector.h>
#include <utility>
#include <algorithm>
#include <limits>
#include <vector>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

/** Store the lowest n items seen so far.
 */
template <class Score, class Data, class Less=std::less<Score> >
class MinimalSet {
  unsigned int n_;
  typedef std::pair<Score, Data> MP;
  typedef base::Vector<MP> Vector;
  Vector data_;

  struct CompareFirst {
    Less l_;
    template <class T>
    bool operator()(const T &a, const T &b) const {
      return l_(a.first, b.first);
    }
  };
  CompareFirst cf_;
  Less l_;

public:
  MinimalSet(unsigned int n): n_(n){}

  bool can_insert(Score s) const {
    if (data_.size() < n_) return true;
    else return l_(s, data_.back().first);
  }

  void insert(Score s, const Data &d) {
    // no-op if set is already filled
    if (!can_insert(s)) {
      // Note: previously this was an assert; however
      // if (can_insert(s)) { insert(s,d); } can fail if s~data.back.first,
      // if floating point rounding occurs, s is placed into a register, etc.
      return;
    }
    std::pair<Score, Data> pair(s, d);
    typename Vector::iterator it=
      std::upper_bound(data_.begin(), data_.end(), pair,
                       CompareFirst());
    data_.insert(it, pair);
    if (size() > n_) {
      data_.pop_back();
    }
  }

  unsigned int size() const {
    return data_.size();
  }

  const MP &operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < data_.size(), "Index out of range in MinimalSet");
    return data_[i];
  }

  void show(std::ostream &out= std::cout) const {
    for (unsigned int i=0; i<n_; ++i) {
      out << data_[i].first << " ";
    }
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_MINIMAL_SET_H */

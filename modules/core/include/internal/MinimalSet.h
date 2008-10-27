/**
 *  \file MinimalSet.h    \brief Maintain a set of low scoring elements.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_MINIMAL_SET_H
#define IMPCORE_INTERNAL_MINIMAL_SET_H

#include "../core_exports.h"
#include "../macros.h"

#include <utility>
#include <algorithm>
#include <limits>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

/** Store the lowest n items seen so far.
 */
template <class Score, class Data>
class MinimalSet {
  unsigned int n_;
  typedef std::pair<Score, Data> MP;
  typedef std::vector<MP> Vector;
  Vector data_;

  struct CompareFirst {
    template <class T>
    bool operator()(const T &a, const T &b) const {
      return a.first < b.first;
    }
  };

public:
  MinimalSet(unsigned int n): n_(n){}

  bool can_insert(Score s) const {
    if (data_.size() < n_) return true;
    else return data_.back().first > s;
  }

  void insert(Score s, const Data &d) {
    IMP_assert(can_insert(s), "Invalid insert");
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
    return data_[i];
  }

};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_MINIMAL_SET_H */

/**
 * \file Distribution.h \brief computes
 *
 * base distribution class
 * required for calculation of SAXS profile and SAXS chi-square derivates
 *
 * Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_DISTRIBUTION_H
#define IMPSAXS_DISTRIBUTION_H

#include "config.h"
#include <IMP/algebra/utility.h>

IMPSAXS_BEGIN_NAMESPACE

template<class ValueT>
class IMPSAXSEXPORT Distribution {
public:
  Distribution(Float bin_size) : bin_size_(bin_size) {}

  void add_to_distribution(Float dist, ValueT& value);

  // ! get distribution as an array of ValueT
  std::vector<ValueT> get_distribution() const { return distribution_; }
  Float get_max_pr_distance(void) { return max_pr_distance_; }
  Float get_bin_size_(void) { return bin_size_; }

//protected:
  unsigned int dist2index(Float dist) const {
    return algebra::round( dist/bin_size_ );
  }

  Float index2dist(unsigned int index) const { return index*bin_size_; }

//protected:
public:
  std::vector<ValueT> distribution_;
  Float bin_size_; // resolution of discretization
  Float max_pr_distance_;  // paramter for maximum r value for p(r) function
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DISTRIBUTION_H */

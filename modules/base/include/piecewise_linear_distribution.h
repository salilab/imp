/**
 *  \file IMP/base/piecewise_linear_distribution.h
 *  \brief boost piecewise linear.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_PIECEWISE_LINEAR_DISTRIBUTION_H
#define IMPBASE_PIECEWISE_LINEAR_DISTRIBUTION_H

#include <IMP/base/base_config.h>
#include "Vector.h"
#include <boost/version.hpp>
#include <boost/random/uniform_real.hpp>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <stdexcept>

IMPBASE_BEGIN_NAMESPACE

/** Draw random numbers from a distribution defined as a piecewise
    linear function. It models the boost random number generators
    and is made redundant by the boost::piecewise_linear_distribution in
    boost 1.47. Currently, it won't use that class if available, but
    that is just due to lack of boost 1.47 to test on.
*/
template <class T=double>
class piecewise_linear_distribution {
  Vector<T> dividers_;
  Vector<T> accum_;
  Vector<T> weights_;
  mutable boost::uniform_real<> un_;
  double get_divider(int divider) const {
    return dividers_[divider];
  }
  double get_accum(int divider) const {
    return accum_[divider];
  }
  double get_slope(int divider) const {
    return (weights_[divider+1]-weights_[divider])
        /(dividers_[divider+1]-dividers_[divider]);
  }
  double get_weight(int divider) const {
    return weights_[divider];  }
 public:
  //! construct a uniform 01
  piecewise_linear_distribution(): un_(0,1) {
    dividers_.push_back(0);
    dividers_.push_back(1);
    accum_.push_back(0);
    accum_.push_back(1);
    weights_.push_back(1);
    weights_.push_back(1);
  }
  /** Construct the distribution by interpolating between samples
      at locations[i] with corresponding weight weights[i]. The
      weight outside of the first and last location is assume to
      be immediately 0.
  */
  template <class LIt, class WIt>
      piecewise_linear_distribution(LIt locations_begin, LIt locations_end,
                                    WIt weights_begin) :
      dividers_(locations_begin,
                locations_end),
      accum_(std::distance(locations_begin, locations_end)),
      weights_(std::distance(locations_begin, locations_end)),
      un_(0,1) {
    for (unsigned int i=0; i< weights_.size(); ++i) {
      weights_[i]=*weights_begin;
      ++weights_begin;
    }
    accum_[0]=0;
    for (unsigned int i=1; i< accum_.size(); ++i) {
      double wid=(dividers_[i]-dividers_[i-1]);
      accum_[i]= accum_[i-1]+wid*.5*(weights_[i-1]+weights_[i]);
    }
    double total=accum_.back();
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    for (unsigned int i=1; i< dividers_.size(); ++i) {
      if (dividers_[i] <= dividers_[i-1]) {
        std::cerr << "Found non-monotonic locations: ";
        std::copy(locations_begin, locations_end,
                  std::ostream_iterator<double>(std::cerr, ", "));
        std::cerr << std::endl;
        throw std::logic_error("Locations are not strictly increasing");
      }
    }
    // check that the total weight is ~1
    if (std::abs(total-1.0) > .05){
      std::cerr << "Found weight " << total << std::endl;
      std::cerr << "Locations: [";
      std::copy(locations_begin, locations_end,
                std::ostream_iterator<double>(std::cerr, ", "));
      std::cerr << "]" << std::endl;
      std::cerr << "Weights: [";
      std::copy(weights_.begin(), weights_.end(),
                std::ostream_iterator<double>(std::cerr, ", "));
      std::cerr << "]" << std::endl;
      throw
          std::logic_error
          ("The total weight of the distribution is not close to 1");
    }
 #endif
    // correct for discretization errors
    for (unsigned int i=0; i< accum_.size(); ++i) {
      accum_[i]/=total;
    }
    /*std::cout << "dividers: ";
    std::copy(dividers_.begin(), dividers_.end(),
              std::ostream_iterator<double>(std::cout, ", "));
    std::cout << std::endl << "sums: ";
    std::copy(accum_.begin(), accum_.end(),
              std::ostream_iterator<double>(std::cout, ", "));
    std::cout << std::endl;
    std::cout << "weights: ";
    std::copy(weights_.begin(), weights_.end(),
              std::ostream_iterator<double>(std::cout, ", "));
              std::cout << std::endl;*/
  }
  template <class RNG>
      double operator()(RNG& rng) const {
    double rn= un_(rng);
    typename Vector<T>::const_iterator it
        = std::upper_bound(accum_.begin()+1, accum_.end(), rn);
    if (it == accum_.end()) {
      std::cerr << "Hit end " << rn << " " << accum_.back() << std::endl;
      return dividers_.back();
    } else {
      int offset= it-accum_.begin()-1;
      double pd= get_weight(offset);
      double ad= get_accum(offset);
      double sd= get_slope(offset);
      double leftd=rn-ad;
      double radix=std::sqrt(pd*pd+2*leftd*sd);
      double xmd;
      if (sd < 0) {
        xmd= (-pd + radix)/sd;
      } else {
        xmd= (-pd + radix)/(sd);
      }
#ifndef NDEBUG
      if (xmd <0) {
        std::cerr << "negative " << std::endl;
      }
#endif
      /*std::cout << rn << " " << pd <<  " " << ad << " " << sd << " "
        << leftd << " " << xmd << std::endl;*/
      return xmd+get_divider(offset);
    }
  }
};



IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_PIECEWISE_LINEAR_DISTRIBUTION_H */

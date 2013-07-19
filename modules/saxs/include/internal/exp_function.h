/**
 * \file ExpFunction
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_INTERNAL_EXP_FUNCTION_H
#define IMPSAXS_INTERNAL_EXP_FUNCTION_H

#include <IMP/saxs/saxs_config.h>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

class ExpFunction : public std::vector<float> {
public:
  // Constructor
  ExpFunction(float max_value, float bin_size) {
    bin_size_ = bin_size;
    one_over_bin_size_ = 1.0/bin_size_;
    max_value_ = max_value;
    unsigned int size = value2index(max_value_) + 1;
    reserve(size);
    for(unsigned int i=0; i<=size; i++) {
      float x = i*bin_size_;
      push_back(std::exp(x));
    }
  }

  unsigned int value2index(float value) const {
    return IMP::algebra::get_rounded(value * one_over_bin_size_ );
  }

  // get exp value for x, compute values if they weren't computed yet
  float exp(float x) {
    unsigned int index = value2index(std::abs(x));
    if(index >= size()) {
      reserve(index);
      for(unsigned int i=size(); i<=index; i++) {
        float x = i*bin_size_;
        push_back(std::exp(x));
      }
    }
    if(x<0.0) return 1.0/(*this)[index];
    return (*this)[index];
  }

private:
  float bin_size_, one_over_bin_size_; // resolution of discretization
  float max_value_;
};

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_EXP_FUNCTION_H */

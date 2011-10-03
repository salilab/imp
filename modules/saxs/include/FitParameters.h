/**
 * \file FitParameters \brief
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSAXS_FIT_PARAMETERS_H
#define IMPSAXS_FIT_PARAMETERS_H

#include <iostream>

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT FitParameters {
 public:
  FitParameters(float chi, float c1, float c2, float c, float o):
    chi_(chi), c1_(c1), c2_(c2), c_(c), o_(o) {}
  FitParameters(float chi, float c1, float c2):
    chi_(chi), c1_(c1), c2_(c2), c_(0.0), o_(0.0) {}

  float get_chi() const { return chi_; }
  float get_c1() const { return c1_; }
  float get_c2() const { return c2_; }
  float get_scale() const { return c_; }
  float get_offset() const { return o_; }

  void show(std::ostream& s) const {
    s << "Chi = " << chi_ << " c1 = " << c1_ << " c2 = " << c2_ << std::endl;
  }
 private:
  float chi_; // fit score
  float c1_; // excluded volume fit
  float c2_; // water layer fit
  float c_; // scaling
  float o_; // offset
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_FIT_PARAMETERS_H */

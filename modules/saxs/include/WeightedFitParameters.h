/**
 * \file IMP/saxs/WeightedFitParameters.h
 *
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_WEIGHTED_FIT_PARAMETERS_H
#define IMPSAXS_WEIGHTED_FIT_PARAMETERS_H

#include <IMP/saxs/saxs_config.h>

#include "FitParameters.h"
#include <IMP/Vector.h>

#include <iostream>

IMPSAXS_BEGIN_NAMESPACE

//! Parameters of a weighted fit, from WeightedProfileFitter.
class WeightedFitParameters : public FitParameters {
 public:
  WeightedFitParameters() : FitParameters() {}

  WeightedFitParameters(double chi, double c1, double c2,
                        const Vector<double>& weights)
      : FitParameters(chi, c1, c2), weights_(weights) {}

  const Vector<double>& get_weights() const { return weights_; }

  void set_weights(const Vector<double>& weights) { weights_ = weights; }

  void show(std::ostream& s) const {
    s << "Chi = " << chi_ << " c1 = " << c1_ << " c2 = " << c2_
      << " default chi = " << default_chi_ << std::endl;
  }

 private:
  Vector<double> weights_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_WEIGHTED_FIT_PARAMETERS_H */

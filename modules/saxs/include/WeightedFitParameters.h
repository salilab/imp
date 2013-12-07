/**
 * \file IMP/saxs/WeightedFitParameters.h \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_WEIGHTED_FIT_PARAMETERS_H
#define IMPSAXS_WEIGHTED_FIT_PARAMETERS_H

#include <IMP/saxs/saxs_config.h>

#include "FitParameters.h"

#include <IMP/base_types.h>

#include <iostream>

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT WeightedFitParameters : public FitParameters {
 public:
  WeightedFitParameters(float chi, float c1, float c2, Floats weights)
      : FitParameters(chi, c1, c2), weights_(weights) {}

  const Floats& get_weights() const { return weights_; }

  void show(std::ostream& s) const {
    s << "Chi = " << chi_ << " c1 = " << c1_ << " c2 = " << c2_
      << " default chi = " << default_chi_ << std::endl;
  }

 private:
  Floats weights_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_WEIGHTED_FIT_PARAMETERS_H */

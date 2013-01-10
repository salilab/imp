/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/ScoreAccumulator.h"
#include "IMP/Restraint.h"
IMP_BEGIN_NAMESPACE
ScoreAccumulator::ScoreAccumulator(ScoreAccumulator o,
                                   const Restraint *r) {
  score_=o.score_;
  weight_= DerivativeAccumulator(o.weight_, r->get_weight());
  deriv_=o.deriv_;
  abort_on_bad_=o.abort_on_bad_;
  global_max_= o.global_max_;
  local_max_= std::min(o.local_max_/weight_.get_weight(),
                       r->get_maximum_score());
}

IMP_END_NAMESPACE

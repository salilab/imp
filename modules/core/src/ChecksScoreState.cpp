/**
 *  \file ChecksScoreState.cpp
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#include <IMP/core/ChecksScoreState.h>
#include <IMP/Pointer.h>
#include <IMP/exception.h>
#include <IMP/random.h>
#include <boost/random/uniform_real_distribution.hpp>

IMPCORE_BEGIN_NAMESPACE

ChecksScoreState::ChecksScoreState(Model *m, double prob)
    : ScoreState(m, "ChecksScoreState %1%"),
      probability_(prob),
      num_checked_(0) {
  IMP_USAGE_CHECK(prob >= 0 && prob <= 1,
                  "Probability must be a number between 0 and 1.");
}

void ChecksScoreState::do_before_evaluate() {
  ::boost::random::uniform_real_distribution<> rand(0, 1);
  if (rand(random_number_generator) < probability_) {
    set_check_level(USAGE_AND_INTERNAL);
    ++num_checked_;
  } else {
    set_check_level(NONE);
  }
}
void ChecksScoreState::do_after_evaluate(DerivativeAccumulator *) {}

ModelObjectsTemp ChecksScoreState::do_get_inputs() const {
  return ModelObjectsTemp();
}

ModelObjectsTemp ChecksScoreState::do_get_outputs() const {
  return ModelObjectsTemp();
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::ChecksScoreState);

IMPCORE_END_NAMESPACE
